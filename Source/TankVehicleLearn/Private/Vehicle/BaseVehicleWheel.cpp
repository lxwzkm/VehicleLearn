// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/BaseVehicleWheel.h"

#include "Kismet/KismetMathLibrary.h"

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
	
}

void UBaseVehicleWheel::GetSupportVector(const float DeltaTime, float& SupportForce, float& SpringForce,
	float& DampForce, FVector& ForceVector)
{
	
}

bool UBaseVehicleWheel::IsZeroPressure()
{
	return true;
}

FRotator UBaseVehicleWheel::GetSuspensionRotation()
{
	return UKismetMathLibrary::TransformRotation(ParentVehicleBody->GetComponentTransform(),WheelInitRelTransf.Rotator());
}

void UBaseVehicleWheel::WheelRolling(const float DeltaTime, const float PowerValue,bool bForceRolling)
{
	
}

void UBaseVehicleWheel::SetWheelSteerAngle(const float InSteerAngle)
{
	
}
