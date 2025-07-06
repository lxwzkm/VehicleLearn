// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/BaseVehiclePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnitConversion.h"
#include "Vehicle/BaseVehicleWheel.h"


ABaseVehiclePawn::ABaseVehiclePawn()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	VehicleBody=CreateDefaultSubobject<UStaticMeshComponent>("Vehicle Body");
	VehicleBody->SetupAttachment(GetRootComponent());

	

	LeftTrackSpline=CreateDefaultSubobject<USplineComponent>("LeftTrackSpline");
	LeftTrackSpline->SetupAttachment(VehicleBody);
	RightTrackSpline=CreateDefaultSubobject<USplineComponent>("RightTrackSpline");
	RightTrackSpline->SetupAttachment(VehicleBody);
	InstancedTrackMesh=CreateDefaultSubobject<UInstancedStaticMeshComponent>("Instanced Track Mesh");
	InstancedTrackMesh->SetupAttachment(VehicleBody);
	Turrent=CreateDefaultSubobject<UStaticMeshComponent>("Turrent");
	Turrent->SetupAttachment(VehicleBody);
	TurrentGun=CreateDefaultSubobject<UStaticMeshComponent>("TurrentGun");
	TurrentGun->SetupAttachment(Turrent);

	SpringArmComponent=CreateDefaultSubobject<USpringArmComponent>("Spring Arm Component");
	SpringArmComponent->SetupAttachment(VehicleBody);
	CameraComponent=CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
}


void ABaseVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABaseVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateControlInfo(DeltaTime);
	UpdateLandShapInput();
	UpdateSupportForce(DeltaTime);
	UpdateTractionForce();
	UpdateSteeringForce();
	UpdateVehicleMovement(DeltaTime);
	UpdateVehicleDodyState();
}

void ABaseVehiclePawn::InitVehicle()
{
	VehicleMass=VehicleBody->GetMass();
	const float TempMass=VehicleMass/100;

	MaxBreakForce=TempMass * MaxBreakForce * 5.f;
	BreakSpeed=TempMass * BreakSpeed * 5.f;
	const float Rate=GetDriveCount()/VehicleWheels.Num();
	MaxPower=TempMass * MaxPower * Rate ;
	PowerUpSpeed=TempMass * PowerUpSpeed * 1.5 ;
	SuspensionDamp=TempMass * SuspensionDamp * 1.5 ;
	SuspensionStrength=TempMass * SuspensionStrength * 1.5 ;

	for (const auto Wheel:VehicleWheels)
	{
		Wheel->SuspensionDamp=SuspensionDamp;
		Wheel->SuspensionStrength=SuspensionStrength;
	}
}

void ABaseVehiclePawn::ContructWheelInfo()
{
	GetComponents(VehicleWheelClass,VehicleWheels);
	for (const auto Wheel:VehicleWheels)
	{
		const FVector& RelativeLocation=Wheel->GetRelativeLocation();
		const FRotator& RelativeRotation=Wheel->GetRelativeRotation();
		const FVector& UpVector=UKismetMathLibrary::GetUpVector(RelativeRotation);

		const FVector& NewLocation=RelativeLocation+UpVector;
		FTransform NewTransform;
		NewTransform.SetLocation(NewLocation);
		NewTransform.SetRotation(RelativeRotation.Quaternion());
		NewTransform.SetScale3D(FVector(Wheel->GetRelativeScale3D()));
		Wheel->SetInitInfo(NewTransform,VehicleBody,SuspensionDamp,SuspensionStrength);
	}
}

float ABaseVehiclePawn::GetDriveCount()
{
	float DriveCount=0.0f;
	for (const auto Wheel:VehicleWheels)
	{
		if (Wheel->bIsDrivingWheel)
		{
			++DriveCount;
		}
	}
	return DriveCount;
}

void ABaseVehiclePawn::UpdateControlInfo(const float DeltaTime)
{
	//油门状态逐渐归零
	ThrottleUpdateTime+=DeltaTime;
	if (ThrottleUpdateTime>=0.2)
	{
		const float Power=BreakForce>=100?0.1:0.01;
		PowerValue-=(MaxPower*Power);
		PowerValue=FMath::Clamp(PowerValue,0,MaxPower);
	}

	//刹车状态逐渐归零
	BreakUpdateTime+=DeltaTime;
	if (BreakUpdateTime>=0.2)
	{
		BreakForce*=0.9f;
	}

	//车轮转向逐渐恢复归零位置
	SteeringUpdateTime+=DeltaTime;
	if (SteeringUpdateTime>=0.2)
	{
		const float Length=VehicleWheels.Num()<3?0.2f:2.f;
		float SteerNewAngle=SteeringAngle-FMath::Sin(SteeringAngle)*Length;
		SteerNewAngle=FMath::Clamp(SteerNewAngle,MaxSteeringAngle*-1.f,MaxSteeringAngle);
		SteeringAngle=(FMath::Abs(SteerNewAngle)<5.f)?0:SteerNewAngle;
	}
}

void ABaseVehiclePawn::UpdateLandShapInput()
{
	for (const auto Wheel:VehicleWheels)
	{
		Wheel->UpdateSuspensionLength();
	}
}

void ABaseVehiclePawn::UpdateSupportForce(const float DeltaTime)
{
	for (const auto Wheel:VehicleWheels)
	{
		float SupportForce=0.f;
		float SpringForce=0.f;
		float DampForce=0.f;
		FVector ForceVector=FVector::ZeroVector;
		Wheel->GetSupportVector(DeltaTime,SupportForce,SpringForce,DampForce,ForceVector);

		const FVector& Force=SupportForce*ForceVector*FVector(0,0,1);
		const FVector& Location=UKismetMathLibrary::TransformLocation(VehicleBody->GetComponentTransform(),Wheel->WheelInitRelTransf.GetLocation());
		VehicleBody->AddForceAtLocation(Force,Location);
	}
}

void ABaseVehiclePawn::UpdateTractionForce()
{
	if (bIsEngineStart)
	{
		for (const auto Wheel:VehicleWheels)
		{
			UpdateTractionForceAtWheel(Wheel,PowerValue);
		}
	}
}

void ABaseVehiclePawn::UpdateTractionForceAtWheel(UBaseVehicleWheel* InputWheel, const float InputPower,
	const float InputAdjustPower)
{
	
	if (InputWheel->IsZeroPressure()==false)
	{
		//Sequence 1
		FVector TractionForce;
		const FVector WheelPoint=InputWheel->LandImpactPoint;
		if (InputWheel->bIsDrivingWheel)
		{
			float Speed=(GetSpeed()-MaxSpeed-3.f)/5.f;
			Speed=FMath::Clamp(Speed,0.f,1.f);
			float TmpPower=InputPower*GearDirection+InputAdjustPower;
			float Magnitude=TmpPower*100*(1-Speed);
			
			const FVector Direction=UKismetMathLibrary::GetForwardVector(InputWheel->GetSuspensionRotation());
			TractionForce=Direction*Magnitude;
		}
		//Sequence 2
		const FVector Velocity=VehicleBody->GetPhysicsLinearVelocity();
		const float DotVelocity=FVector::DotProduct(Velocity,InputWheel->GetForwardVector());
		const float Mag=FMath::Sin(DotVelocity)*-1;
		const FVector BreakForceOutVector=VehicleBody->GetForwardVector()*Mag;

		float AbsSpeed=FMath::Abs(GetSpeed())/20.f;
		AbsSpeed=FMath::Clamp(AbsSpeed,0.1f,1.f);
		
		const float SteeringFloat=InputWheel->bIsSteeringWheel?50:100;
		const float BreakForceOutMag=BreakForce*AbsSpeed*SteeringFloat;
		
		const FVector BreakForceOut=BreakForceOutVector*BreakForceOutMag;
		const FVector OutForce=BreakForceOut+TractionForce;
		VehicleBody->AddForceAtLocation(OutForce,WheelPoint);
	}
}

float ABaseVehiclePawn::GetSpeed()
{
	return VehicleBody->GetComponentVelocity().Length()*0.036;
}

void ABaseVehiclePawn::UpdateSteeringForce()
{
	//遍历悬挂
	for (const auto Wheel:VehicleWheels)
	{
		//判断阻力是否成立
		if (Wheel->IsZeroPressure()==false)
		{
			//悬挂面的接触发现
			const FVector NormalVector=Wheel->LandImpactNormal;

			//车轮的实际速度
			FVector RealVelocityDirection=VehicleBody->GetPhysicsLinearVelocityAtPoint(Wheel->GetComponentLocation());
			float RealSpeed=RealVelocityDirection.Length();
			RealVelocityDirection.Normalize(0.0001);

			//悬挂右侧向量
			const FVector TargetDirection=UKismetMathLibrary::GetForwardVector(Wheel->GetSuspensionRotation());
			const FVector HDirection=UKismetMathLibrary::GetRightVector(Wheel->GetSuspensionRotation());

			//获取侧向转向阻力
			const FVector DeltaVelocity=(TargetDirection-RealVelocityDirection)*RealSpeed;
			const FVector ProjectVector=ProjectToPlane(NormalVector,DeltaVelocity);
			const FVector TargetCentripetalAcceleration=HDirection*UKismetMathLibrary::Dot_VectorVector(HDirection,ProjectVector);

			const float Mass=VehicleBody->GetMass();
			FVector Acceleration=TargetCentripetalAcceleration*Mass;
			
			const float Length=Acceleration.Length();
			const float MinMag=UKismetMathLibrary::Min(Length,FrictionRate*Mass);
			Acceleration.Normalize(0.0001);
			
			const FVector Force=Acceleration*MinMag;
			VehicleBody->AddForceAtLocation(Force,Wheel->GetComponentLocation());
		}
	}
}

FVector ABaseVehiclePawn::ProjectToPlane(const FVector& NormalVector, const FVector& InputVector)
{
	const float DotProduct=FVector::DotProduct(NormalVector,InputVector);
	return InputVector-(NormalVector*DotProduct);
}

void ABaseVehiclePawn::UpdateVehicleMovement(const float DeltaTime)
{
	UpdateWheelPosition();
	UpdateWheelRolling(DeltaTime);
	UpdateWheelYaw(DeltaTime);
}

void ABaseVehiclePawn::UpdateWheelPosition()
{
}

void ABaseVehiclePawn::UpdateWheelRolling(const float DeltaTime)
{
	for (const auto Wheel:VehicleWheels)
	{
		Wheel->WheelRolling(DeltaTime,PowerValue);
	}
}

void ABaseVehiclePawn::UpdateWheelYaw(const float DeltaTime)
{
	for (const auto Wheel:VehicleWheels)
	{
		if (Wheel->bIsSteeringWheel)
		{
			Wheel->SetWheelSteerAngle(SteeringAngle);
		}
	}
}

void ABaseVehiclePawn::UpdateVehicleDodyState()
{
	const FVector UpDirection=UKismetMathLibrary::GetUpVector(VehicleBody->GetComponentRotation());
	bIsTurnOver=UpDirection.Z<=0;
	if (bIsTurnOver)
	{
		TurnOverEvent();
	}
}

void ABaseVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


