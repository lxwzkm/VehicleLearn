// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "BaseVehicleWheel.generated.h"

/**
 * 
 */
UCLASS()
class TANKVEHICLELEARN_API UBaseVehicleWheel : public UStaticMeshComponent
{
	GENERATED_BODY()
public:

	void SetInitInfo(const FTransform& InWheelInitRelTransf,UStaticMeshComponent* InParentVehicleBody,const float& InSuspensonDamp,const float& InSuspStrength);

	void UpdateSuspensionLength();
	void GetSupportVector(const float DeltaTime,float& SupportForce,float& SpringForce,float& DampForce,FVector& ForceVector);
	bool IsZeroPressure();
	FRotator GetSuspensionRotation();
	void WheelRolling(const float DeltaTime,const float PowerValue,bool bForceRolling=false);
	void SetWheelSteerAngle(const float InSteerAngle);
	void GetPressure(const float DeltaTime,float& pressure,float& SpringForce,float& DampForce);

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ETraceTypeQuery> WheelTraceType;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "SuspensionInfo")
	float NatureSuspensionLength=100.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "SuspensionInfo")
	float SuspensionDamp=100.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "SuspensionInfo")
	float SuspensionStrength=100.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "SuspensionInfo")
	float SuspensionLength=0.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "SuspensionInfo")
	float LastSuspensionLength=0.f;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "VehicleInfo")
	FTransform WheelInitRelTransf=FTransform();
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "VehicleInfo")
	TObjectPtr<UStaticMeshComponent>ParentVehicleBody;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "WheelInfo")
	bool bIsDrivingWheel=true;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "WheelInfo")
	bool bIsSteeringWheel=false;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "WheelInfo")
	FVector LandImpactPoint=FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "WheelInfo")
	FVector LandImpactNormal=FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "WheelInfo")
	float WheelRadius=45.f;

protected:
};
