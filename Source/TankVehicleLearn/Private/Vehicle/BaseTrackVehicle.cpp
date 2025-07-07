// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/BaseTrackVehicle.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Vehicle/BaseVehicleWheel.h"

void ABaseTrackVehicle::ConstructTracksSpline(const TArray<UBaseVehicleWheel*>& WheelList,
                                              USplineComponent* InputTrackSpline, TMap<int32, UBaseVehicleWheel*>& WheelSplineIndexMap,
                                              TMap<int32, FVector>& WheelPositionDiffMap)
{
	//Sequence 1
	int32 Length=WheelList.Num();
	TArray<FVector> TmpLinePointsList;
	for (int32 i = 0; i < Length; i++)
	{
		int32 TmpIndex=i%WheelList.Num();
		int32 NextIndex=(TmpIndex+1)%WheelList.Num();

		const FVector CurrentLocation=WheelList[TmpIndex]->GetComponentLocation();
		const float CurrentRadius=WheelList[TmpIndex]->WheelRadius+TrackThick;
		const FVector NextLocation=WheelList[NextIndex]->GetComponentLocation();
		const float NextRadius=WheelList[NextIndex]->WheelRadius+TrackThick;

		TArray<int32> WheelPoints;
		TArray<int32> NextWheelPoints;
		InsertPointToSpline(CurrentLocation,CurrentRadius,NextLocation,NextRadius,TmpLinePointsList,WheelPoints,NextWheelPoints);
		for (int32 Point:WheelPoints)
		{
			WheelSplineIndexMap.Add(Point,WheelList[TmpIndex]);
		}
		for (int32 Point:NextWheelPoints)
		{
			WheelSplineIndexMap.Add(Point,WheelList[NextIndex]);
		}
	}

	//Sequence 2
	const FVector StartPoint=TmpLinePointsList.Last();
	const FVector EndPoint=TmpLinePointsList.Top();
	const FVector Center=WheelList[0]->GetComponentLocation();
	const float Radius=WheelList[0]->WheelRadius+TrackThick;
	TArray<FVector> InsertPoints;
	InsertPointsOnWheel(StartPoint,EndPoint,Center,Radius,InsertPoints);
	for (FVector Point:InsertPoints)
	{
		int32 Index=TmpLinePointsList.Add(Point);
		WheelSplineIndexMap.Add(Index,WheelList[0]);
	}
	const FVector TmpVector=TmpLinePointsList.Top();
	int32 TmpIndewx=TmpLinePointsList.Add(TmpVector);
	WheelSplineIndexMap.Add(TmpIndewx,WheelList[0]);

	//Sequence 3
	Length=TmpLinePointsList.Num();
	for (int32 i=0;i<TmpLinePointsList.Num();i++)
	{
		const FVector DiffVector=TmpLinePointsList[i]-WheelSplineIndexMap[i]->GetComponentLocation();
		WheelPositionDiffMap.Add(i,DiffVector);
	}

	//Sequence 4
	TArray<FVector> LinePointsList;
	for (int32 i=0;i<TmpLinePointsList.Num();i++)
	{
		const FVector LinePoint=UKismetMathLibrary::InverseTransformLocation(InputTrackSpline->GetComponentTransform(),TmpLinePointsList[i]);
		LinePointsList.Add(LinePoint);
	}
	InputTrackSpline->SetSplinePoints(LinePointsList,ESplineCoordinateSpace::Local,true);
	for (int32 i=0;i<LinePointsList.Num();i++)
	{
		int32 TmpIndex=(Length+i)%Length;
		int32 NextIndex=(TmpIndex+1+Length)%Length;
		int32 PreIndex=(TmpIndex-1+Length)%Length;

		const FVector Arrive=(LinePointsList[TmpIndex]-LinePointsList[PreIndex])*0.25;
		const FVector Leave=(LinePointsList[NextIndex]-LinePointsList[TmpIndex])*0.25;
		InputTrackSpline->SetTangentsAtSplinePoint(TmpIndex,Arrive,Leave,ESplineCoordinateSpace::Local,true);
	}
}

void ABaseTrackVehicle::AddTrackStaticmesh(USplineComponent* InputTrackSpline,
	UInstancedStaticMeshComponent* InputInstancedTrackMesh, TMap<int32, float>& InTrackPartSplinePositionMap)
{
	//Sequence 1
	float PerTrackLength=InputTrackSpline->GetSplineLength()/TrackNumber;

	//Sequence 2
	for (int32 i=0;i<TrackNumber;i++)
	{
		float TmpSplinePosition=i*PerTrackLength;
		const FVector SplineLocation=InputTrackSpline->GetTransformAtDistanceAlongSpline(TmpSplinePosition,ESplineCoordinateSpace::Local,false).GetLocation();
		FRotator SplineRotation=InputTrackSpline->GetRotationAtDistanceAlongSpline(TmpSplinePosition,ESplineCoordinateSpace::Local);
		const FVector SplineRightVector=InputTrackSpline->GetRightVectorAtDistanceAlongSpline(TmpSplinePosition,ESplineCoordinateSpace::Local);
		SplineRotation.Roll=TrackPartRot+SplineRightVector.Y<0?180.f:SplineRotation.Roll;
		FTransform SplineTransform;
		SplineTransform.SetLocation(SplineLocation);
		SplineTransform.SetRotation(SplineRotation.Quaternion());
		InputInstancedTrackMesh->UpdateInstanceTransform(i,SplineTransform,false,true,false);
	}
}

void ABaseTrackVehicle::InsertPointToSpline(const FVector Wheel, const float Radius,
	const FVector WheelNext, const float RadiusNext, TArray<FVector>& TmpLinePointsList,TArray<int32>& WheelPointOut,TArray<int32>& NextWheelPointOut)
{
	//Sequence 1
	const FVector Forward=VehicleBody->GetForwardVector();
	const FVector DownDirection=VehicleBody->GetUpVector()*-1.f;
	TArray<FVector> TmpTanPoints=GetTanBetweenWheels(Wheel,Radius,WheelNext,RadiusNext,DownDirection,Forward);

	//Sequence 2
	if (TmpLinePointsList.Num()>0)
	{
		TArray<FVector> InsertPoints;
		InsertPointsOnWheel(TmpLinePointsList.Last(),TmpTanPoints[0],Wheel,Radius,InsertPoints);

		//Sequence 3
		for (FVector Point:InsertPoints)
		{
			int32 Index=TmpLinePointsList.Add(Point);
			WheelPointOut.Add(Index);
		}
		for (int32 i=0;i<TmpTanPoints.Num();i++)
		{
			int32 TanIndex=TmpLinePointsList.Add(TmpTanPoints[i]);
			if (i==0)
			{
				WheelPointOut.Add(TanIndex);
			}
			else
			{
				NextWheelPointOut.Add(TanIndex);
			}
		}
	}
	
}

void ABaseTrackVehicle::InsertPointsOnWheel(const FVector InStartPoint, const FVector InEndPoint,
	const FVector InCenter, const float InWheelRadius, TArray<FVector>& OutInsertPoints)
{
	const float DeltaAngle=15.f;
	const FVector StartVector=InStartPoint-InCenter;
	const FVector EndVector=InEndPoint-InCenter;

	const float DotValue=UKismetMathLibrary::Dot_VectorVector(StartVector,EndVector);
	if (const float DiffAngle=UKismetMathLibrary::Acos(StartVector.Length()*EndVector.Length()/DotValue); FMath::Abs(DiffAngle)>=DeltaAngle)
	{
		FVector Direction=StartVector+EndVector;
		Direction.Normalize(0.0001);
		const FVector MidPoint=InCenter+Direction*InWheelRadius;

		InsertPointsOnWheel(InStartPoint,MidPoint,InCenter,InWheelRadius,OutInsertPoints);
		OutInsertPoints.Add(MidPoint);
		InsertPointsOnWheel(MidPoint,InEndPoint,InCenter,InWheelRadius,OutInsertPoints);
	}
}

TArray<FVector> ABaseTrackVehicle::GetTanBetweenWheels(const FVector Incenter1,const float InRadius1,const FVector Incenter2,const float InRadius2,FVector InDownDirction,const FVector InForwardDirection)
{
	TArray<FVector> TmpTanPoints;
	const float DiffRadius=InRadius2-InRadius1;
	const FVector CenterDirection=Incenter2-Incenter1;
	if (UKismetMathLibrary::Dot_VectorVector(CenterDirection,InForwardDirection)<0)
	{
		InDownDirction*=-1;
	}
	const float CenterDist=CenterDirection.Length();
	FVector RadiusDirection=UKismetMathLibrary::Cross_VectorVector(UKismetMathLibrary::Cross_VectorVector(CenterDirection,InDownDirction),CenterDirection);
	RadiusDirection.Normalize(0.0001);
	const float TanAngle=UKismetMathLibrary::Asin(DiffRadius/CenterDist);
	FVector TanDirection=RadiusDirection*(UKismetMathLibrary::Tan(TanAngle)*CenterDist)+CenterDirection;
	TanDirection.Normalize(0.0001);
	const FVector TanVector=UKismetMathLibrary::Cos(TanAngle)*CenterDist*TanDirection;

	const FVector TanPoint1=Incenter1+(RadiusDirection*InRadius1);
	const FVector TanPoint2=TanPoint1+TanVector;
	TmpTanPoints.Add(TanPoint1);
	TmpTanPoints.Add(TanPoint2);
	return TmpTanPoints;
}

void ABaseTrackVehicle::CompleteConstructTarack()
{
	for (auto Wheel:LeftWheels)
	{
		Wheel->WheelRadius+=TrackThick;
	}
	for (auto Wheel:RightWheels)
	{
		Wheel->WheelRadius-=TrackThick;
	}
	if (VehicleBody->DoesSocketExist("TurretSocket"))
	{
		Turrent->SetWorldLocation(VehicleBody->GetSocketLocation("TurretSocket"));
	}
	if (Turrent->DoesSocketExist("GunSocket"))
	{
		TurrentGun->SetWorldLocation(Turrent->GetSocketLocation("GunSocket"));
	}
}
