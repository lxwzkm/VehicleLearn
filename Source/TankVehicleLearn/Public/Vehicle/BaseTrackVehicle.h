// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Vehicle/BaseVehiclePawn.h"
#include "BaseTrackVehicle.generated.h"

/**
 * 
 */
UCLASS()
class TANKVEHICLELEARN_API ABaseTrackVehicle : public ABaseVehiclePawn
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void RegistTrackWheel();

	UFUNCTION(BlueprintCallable)
	void ConstructTracksSpline(const TArray<UBaseVehicleWheel*>& WheelList,UPARAM(ref)USplineComponent* InputTrackSpline,
		UPARAM(ref)TMap<int32,UBaseVehicleWheel*>& WheelSplineIndexMap,UPARAM(ref)TMap<int32,FVector>& WheelPositionDiffMap);
	UFUNCTION(BlueprintCallable)
	void AddTrackStaticmesh(UPARAM(ref)USplineComponent* InputTrackSpline,UInstancedStaticMeshComponent* InputInstancedTrackMesh,
		UPARAM(ref)TMap<int32,float>& InTrackPartSplinePositionMap);
	void InsertPointToSpline(const FVector Wheel,const float Radius,const FVector WheelNext,const float RadiusNext,UPARAM(ref)TArray<FVector>& TmpLinePointsList,
		TArray<int32>& WheelPointOut,TArray<int32>& NextWheelPointOut);
	void InsertPointsOnWheel(const FVector& InStartPoint,const FVector& InEndPoint,const FVector& InCenter,const float InWheelRadius,TArray<FVector>& OutInsertPoints);
	TArray<FVector> GetTanBetweenWheels(const FVector Incenter1,const float InRadius1,const FVector Incenter2,const float InRadius2,FVector InDownDirction,const FVector InForwardDirection);

	virtual void UpdateVehicleMovement(const float DeltaTime) override;
	virtual void UpdateWheelRolling(const float DeltaTime) override;
	void SideWheelRolling(const float DeltaTime,TArray<UBaseVehicleWheel*>& WheelList);
	void UpdateTrackMovement(USplineComponent* InputTrackSpline,const UInstancedStaticMeshComponent* InputInstancedTrackMesh,const TArray<UBaseVehicleWheel*>& WheelList,TMap<int32,UBaseVehicleWheel*>& InWheelSplineMap
		,TMap<int32,FVector>& InWheelPositionDiffMap,TMap<int32,float>& TrackPartSplinePositionMap,float DeltaTime,bool bIsLeftPart=false);
	UFUNCTION(BlueprintCallable)
	void CompleteConstructTarack();
	virtual void UpdateSteeringForce() override;
	virtual void InitVehicle() override;
	
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,UBaseVehicleWheel*> LeftWheelSplineMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,UBaseVehicleWheel*> RightWheelSplineMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,FVector> LeftWheelPositionDiffMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,FVector> RightWheelPositionDiffMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,float> LeftTrackPartSplinePositionMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	TMap<int32,float> RightTrackPartSplinePositionMap;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	float LeftTrackMoveDist=0;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="WheelSpline")
	float RightTrackMoveDist=0;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="TrackInfo")
	float TrackThick=10.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="TrackInfo")
	int32 TrackNumber=72;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="TrackInfo")
	float TrackPartRot=180.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="TrackInfo")
	TArray<TObjectPtr<UBaseVehicleWheel>> LeftWheels;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="TrackInfo")
	TArray<TObjectPtr<UBaseVehicleWheel>> RightWheels;
	
};
