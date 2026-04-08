#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeBase.h"
#include "BoneContainer.h"
#include "Engine/Engine.h"
#include "TerrainAdaptationIK.generated.h"

/**
 * Advanced IK system for foot placement on irregular prehistoric terrain
 * Ensures realistic foot contact with rocks, roots, uneven ground
 * 
 * Critical for maintaining immersion - every step must feel grounded and intentional
 */

USTRUCT(BlueprintType)
struct FFootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBoneReference FootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBoneReference IKBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FootOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IKAlpha = 1.0f;

    // Runtime data
    FVector TargetLocation = FVector::ZeroVector;
    FRotator TargetRotation = FRotator::ZeroRotator;
    bool bValidIKTarget = false;
    float LastTraceTime = 0.0f;

    FFootIKData()
    {
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 1.0f;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        bValidIKTarget = false;
        LastTraceTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FTerrainIKSettings
{
    GENERATED_BODY()

    // Trace settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "50.0"))
    float InterpolationSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float MaxSlopeAngle = 45.0f;

    // Foot placement precision
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float FootRadius = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerrainConformity = 0.8f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", ClampMax = "0.5"))
    float TraceFrequency = 0.1f; // Seconds between traces

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableDebugDraw = false;

    FTerrainIKSettings()
    {
        TraceDistance = 50.0f;
        InterpolationSpeed = 15.0f;
        MaxSlopeAngle = 45.0f;
        FootRadius = 5.0f;
        TerrainConformity = 0.8f;
        TraceFrequency = 0.1f;
        bEnableDebugDraw = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTerrainAdaptationIK : public UObject
{
    GENERATED_BODY()

public:
    UTerrainAdaptationIK();

    // Main IK configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
    TArray<FFootIKData> FootIKArray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
    FTerrainIKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup")
    FBoneReference PelvisBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Setup", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PelvisAdjustmentAlpha = 0.5f;

    // Runtime functions
    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void UpdateFootIK(class USkeletalMeshComponent* SkelMeshComp, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FVector GetFootIKOffset(int32 FootIndex) const;

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    FRotator GetFootIKRotation(int32 FootIndex) const;

    UFUNCTION(BlueprintPure, Category = "Terrain IK")
    float GetPelvisOffset() const;

protected:
    // Internal IK processing
    bool PerformFootTrace(const FVector& StartLocation, const FVector& TraceDirection, 
                         FVector& OutHitLocation, FVector& OutHitNormal, UWorld* World);

    void CalculateFootPlacement(FFootIKData& FootData, const FVector& FootWorldLocation, 
                               UWorld* World, float DeltaTime);

    void UpdatePelvisAdjustment(float DeltaTime);

    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal, 
                                            const FRotator& CurrentRotation);

private:
    // Runtime state
    bool bIKEnabled = true;
    float PelvisOffset = 0.0f;
    float TargetPelvisOffset = 0.0f;
    TArray<FVector> FootTraceStarts;
    
    // Performance tracking
    float LastUpdateTime = 0.0f;
    int32 CurrentTraceIndex = 0;
};