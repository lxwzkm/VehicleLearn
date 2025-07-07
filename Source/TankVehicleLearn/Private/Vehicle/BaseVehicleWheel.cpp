// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/BaseVehicleWheel.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UBaseVehicleWheel::SetInitInfo(const FTransform& InWheelInitRelTransf, UStaticMeshComponent* InParentVehicleBody,
                                    const float& InSuspensonDamp, const float& InSuspStrength)
{
	WheelInitRelTransf = InWheelInitRelTransf;
	ParentVehicleBody = InParentVehicleBody;
	SuspensionDamp = InSuspensonDamp;
	SuspensionStrength = InSuspStrength;
}

void UBaseVehicleWheel::UpdateSuspensionLength()
{
	LastSuspensionLength=SuspensionLength;
	const FVector SuspLocation=UKismetMathLibrary::TransformLocation(ParentVehicleBody->GetComponentTransform(),WheelInitRelTransf.GetLocation());
	const FRotator SuspRotation=UKismetMathLibrary::TransformRotation(ParentVehicleBody->GetComponentTransform(),WheelInitRelTransf.Rotator());
	const FVector SuspDirection=UKismetMathLibrary::GetUpVector(SuspRotation);

	FHitResult HitResult;
	const FVector EndLocation=SuspLocation+SuspDirection*-(NatureSuspensionLength+1);
	UKismetSystemLibrary::SphereTraceSingle(this,SuspLocation,EndLocation,WheelRadius,WheelTraceType,false
		,TArray<AActor*>(),EDrawDebugTrace::None,HitResult,true);

	if (HitResult.bBlockingHit)
	{
		LandImpactPoint=HitResult.ImpactPoint;
		LandImpactNormal=HitResult.ImpactNormal;
		SuspensionLength=(SuspLocation-LandImpactPoint).Length()-WheelRadius;
	}
	else
	{
		SuspensionLength=NatureSuspensionLength;
	}

	const FVector NewLocation=SuspLocation+SuspDirection*-SuspensionLength;
	SetWorldLocation(NewLocation);
}

void UBaseVehicleWheel::GetSupportVector(const float DeltaTime, float& SupportForce, float& SpringForce,
	float& DampForce, FVector& ForceVector)
{
	GetPressure(DeltaTime,SupportForce,SpringForce,DampForce);
	const FVector UpVector=UKismetMathLibrary::GetUpVector(WheelInitRelTransf.Rotator());
	ForceVector=UKismetMathLibrary::TransformDirection(WheelInitRelTransf,UpVector);
	
}

void UBaseVehicleWheel::GetPressure(const float DeltaTime, float& pressure, float& SpringForce, float& DampForce)
{
	float TempMaxForce=NatureSuspensionLength*SuspensionLength;

	SpringForce=(NatureSuspensionLength-SuspensionLength)*SuspensionStrength;
	SpringForce=FMath::Clamp(SpringForce,0,TempMaxForce);

	DampForce=(SuspensionLength-LastSuspensionLength)/DeltaTime*SuspensionDamp*-1;

	pressure=SpringForce+DampForce;
	pressure=FMath::Clamp(pressure,-TempMaxForce,TempMaxForce);
}

bool UBaseVehicleWheel::IsZeroPressure()
{
	return NatureSuspensionLength-SuspensionLength==0.f;
}

FRotator UBaseVehicleWheel::GetSuspensionRotation()
{
	return UKismetMathLibrary::TransformRotation(ParentVehicleBody->GetComponentTransform(),WheelInitRelTransf.Rotator());
}

void UBaseVehicleWheel::WheelRolling(const float DeltaTime, const float PowerValue,bool bForceRolling)
{
	//Sequence 1
	const bool IsNotZeroPressure=IsZeroPressure()==false;
	const bool IsThrottleUp=PowerValue>0 && bIsDrivingWheel;
	float RotSpeed=0.f;
	if (IsNotZeroPressure || IsThrottleUp || bForceRolling)
	{
		//正在行驶
		const FVector& VelocityPoint=ParentVehicleBody->GetPhysicsLinearVelocityAtPoint(GetComponentLocation());
		const float SuspensionSpeed=VelocityPoint.Size2D();
		
		const FVector ForwardVector=UKismetMathLibrary::GetForwardVector(ParentVehicleBody->GetComponentRotation());
		const float DotValue=UKismetMathLibrary::Dot_VectorVector(ForwardVector,VelocityPoint);
		const float EdgeSpeed=DotValue*SuspensionSpeed;

		RotSpeed=EdgeSpeed/WheelRadius*-1*57.310001;
	}
	else
	{
		RotSpeed*=0.98;
	}

	//Sequence 2
	AddLocalRotation(FRotator(0,RotSpeed*DeltaTime,0),true);
}

void UBaseVehicleWheel::SetWheelSteerAngle(const float InSteerAngle)
{
	FRotator WheelRotation=WheelInitRelTransf.Rotator();
	WheelRotation.Yaw=InSteerAngle;
	WheelInitRelTransf.SetRotation(WheelRotation.Quaternion());

	FRotator TemRelativeRotation=GetRelativeRotation();
	if (FMath::Abs(TemRelativeRotation.Yaw)>0.01f)
	{
		TemRelativeRotation.Yaw=InSteerAngle+TemRelativeRotation.Yaw>=0?0:180;
		SetRelativeRotation(TemRelativeRotation);
	}
}
