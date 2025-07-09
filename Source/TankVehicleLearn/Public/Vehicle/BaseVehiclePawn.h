// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehiclePawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USplineComponent;
class UBaseVehicleWheel;

UCLASS()
class TANKVEHICLELEARN_API ABaseVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	ABaseVehiclePawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	virtual void InitVehicle();
	UFUNCTION(BlueprintCallable)
	void ContructWheelInfo();
	float GetDriveCount();

	void UpdateControlInfo(const float DeltaTime);
	void UpdateLandShapInput();
	void UpdateSupportForce(const float DeltaTime);
	void UpdateTractionForce();
	void UpdateTractionForceAtWheel(UBaseVehicleWheel* InputWheel,const float InputPower,const float InputAdjustPower=0.f);
	float GetSpeed();
	virtual void UpdateSteeringForce();
	FVector ProjectToPlane(const FVector&NormalVector,const FVector& InputVector);
	virtual void UpdateVehicleMovement(const float DeltaTime);
	void UpdateWheelPosition();
	virtual void UpdateWheelRolling(const float DeltaTime);
	void UpdateWheelYaw(const float DeltaTime);
	void UpdateVehicleDodyState();
	

	UFUNCTION(BlueprintImplementableEvent)
	void TurnOverEvent();
	UFUNCTION(BlueprintImplementableEvent)
	void EnginStart();
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Vehicle")
	TObjectPtr<UStaticMeshComponent> VehicleBody;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Vehicle")
	TObjectPtr<USplineComponent> LeftTrackSpline;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Vehicle")
	TObjectPtr<USplineComponent> RightTrackSpline;
	UPROPERTY(EditDefaultsOnly,Category="Vehicle")
	TObjectPtr<UStaticMeshComponent> Turrent;
	UPROPERTY(EditDefaultsOnly,Category="Vehicle")
	TObjectPtr<UStaticMeshComponent> TurrentGun;
	UPROPERTY(EditDefaultsOnly,Category="Vehicle")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(EditDefaultsOnly,Category="Vehicle")
	TObjectPtr<UCameraComponent> CameraComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Vehicle")
	TObjectPtr<UInstancedStaticMeshComponent> InstancedTrackMesh;

	UPROPERTY(EditDefaultsOnly,Category="Vehicle")
	TSubclassOf<UBaseVehicleWheel> VehicleWheelClass;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Vehicle")
	TArray<UBaseVehicleWheel*> VehicleWheels;

	UPROPERTY(EditDefaultsOnly,Category="VehicleInfo")
	float SuspensionDamp=100.f;//悬挂阻尼
	UPROPERTY(EditDefaultsOnly,Category="VehicleInfo")
	float SuspensionStrength=100.f;//悬挂力
	UPROPERTY(EditDefaultsOnly,Category="VehicleInfo")
	float VehicleMass=0.f;
	UPROPERTY(EditDefaultsOnly,Category="VehicleInfo")
	float MaxSpeed=50.f;
	UPROPERTY(EditDefaultsOnly,Category="VehicleInfo")
	float FrictionRate=1000.f;

	UPROPERTY(EditDefaultsOnly,Category="VehicleControlBreak")
	float MaxBreakForce=100.f;
	UPROPERTY(EditDefaultsOnly,Category="VehicleControlBreak")
	float BreakSpeed=1.f;
	UPROPERTY(EditDefaultsOnly,Category="VehicleControlBreak")
	float BreakForce=0.f;

	UPROPERTY(EditDefaultsOnly,Category="VehicleControlSteering")
	float SteeringAngle=0.f;
	UPROPERTY(EditDefaultsOnly,Category="VehicleControlSteering")
	float MaxSteeringAngle=30.f;

	UPROPERTY(EditDefaultsOnly,Category="VehiclePower")
	float MaxPower=100.f;
	UPROPERTY(EditDefaultsOnly,Category="VehiclePower")
	float PowerUpSpeed=1.f;
	UPROPERTY(EditDefaultsOnly,Category="VehiclePower")
	float PowerValue=0.f;
	UPROPERTY(EditDefaultsOnly,Category="VehiclePower")
	int32 GearDirection=0;

	UPROPERTY(EditDefaultsOnly,Category="UpdateTimeRecord")
	float ThrottleUpdateTime=0.f;
	UPROPERTY(EditDefaultsOnly,Category="UpdateTimeRecord")
	float BreakUpdateTime=0.f;
	UPROPERTY(EditDefaultsOnly,Category="UpdateTimeRecord")
	float SteeringUpdateTime=0.f;

	UPROPERTY(EditDefaultsOnly,Category="VehicleState")
	bool bIsEngineStart=false;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="VehicleState")
	bool bIsTurnOver=false;
	
};