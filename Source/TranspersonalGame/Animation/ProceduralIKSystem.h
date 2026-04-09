// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "IKRig.h"
#include "IKRigDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProceduralIKSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProceduralIK, Log, All);

/**
 * IK Target Configuration
 * Defines how IK targets should behave for different limbs
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FIKTargetConfig
{
    GENERATED_BODY()

    // Target bone name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    FName BoneName;

    // IK Goal name in the IK Rig
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    FName IKGoalName;

    // How much this IK affects the final pose (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    float Alpha;

    // How quickly IK adjustments are applied
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    float BlendSpeed;

    // Maximum distance the IK can move from original position
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    float MaxReach;

    // Whether this IK should trace to ground
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tracing")
    bool bTraceToGround;

    // Trace distance for ground detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tracing")
    float TraceDistance;

    // Offset from ground surface
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tracing")
    float GroundOffset;

    FIKTargetConfig()
    {
        BoneName = NAME_None;
        IKGoalName = NAME_None;
        Alpha = 1.0f;
        BlendSpeed = 10.0f;
        MaxReach = 50.0f;
        bTraceToGround = true;
        TraceDistance = 100.0f;
        GroundOffset = 2.0f;
    }
};

/**
 * Foot IK Data
 * Stores current state of foot IK for each foot
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFootIKData
{
    GENERATED_BODY()

    // Current IK target location
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    FVector TargetLocation;

    // Current IK target rotation
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    FRotator TargetRotation;

    // How far the foot is displaced from original position
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    float DisplacementDistance;

    // Surface normal at foot location
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    FVector SurfaceNormal;

    // Whether foot is currently in contact with ground
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    bool bIsGrounded;

    // Current alpha value for blending
    UPROPERTY(BlueprintReadOnly, Category = "IK State")
    float CurrentAlpha;

    FFootIKData()
    {
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        DisplacementDistance = 0.0f;
        SurfaceNormal = FVector::UpVector;
        bIsGrounded = false;
        CurrentAlpha = 0.0f;
    }
};

/**
 * Procedural IK System Component
 * Handles foot IK, hand IK, and other procedural animation adjustments
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralIKSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    class UIKRigDefinition* IKRigAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    TArray<FIKTargetConfig> IKTargets;

    // Foot IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName LeftFootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName RightFootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootGroundOffset;

    // Hand IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    bool bEnableHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FName LeftHandBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FName RightHandBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    float HandIKAlpha;

    // Environmental Adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bAdaptToSlope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float MaxSlopeAngle; // Maximum slope angle to adapt to

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bAdaptToRoughTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float TerrainAdaptationSpeed;

    // Current IK State (read-only for Blueprints)
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FFootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FFootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector LeftHandIKTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector RightHandIKTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentSlopeAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector CurrentSlopeNormal;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency; // How often to update IK (Hz)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 IKLODLevel; // Level of detail for IK calculations

    // Core IK Functions
    UFUNCTION(BlueprintCallable, Category = "Procedural IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Procedural IK")
    void UpdateHandIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Procedural IK")
    void UpdateEnvironmentalAdaptation(float DeltaTime);

    // Foot IK Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool TraceFootToGround(const FVector& FootLocation, FVector& OutGroundLocation, FVector& OutGroundNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector CalculateFootIKTarget(const FVector& FootLocation, const FVector& GroundLocation, const FVector& GroundNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& OriginalRotation);

    // Hand IK Functions
    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void SetHandIKTarget(bool bLeftHand, const FVector& TargetLocation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void ClearHandIKTarget(bool bLeftHand);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    bool TraceHandToSurface(const FVector& HandLocation, const FVector& TraceDirection, FVector& OutSurfaceLocation);

    // Environmental Adaptation Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void AnalyzeTerrain(const FVector& Location, float& OutSlopeAngle, FVector& OutSlopeNormal, float& OutRoughness);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void AdaptToSlope(const FVector& SlopeNormal, float SlopeAngle, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void AdaptToRoughTerrain(float TerrainRoughness, float DeltaTime);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void SetIKAlpha(float Alpha);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void EnableIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetBoneWorldLocation(FName BoneName);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FRotator GetBoneWorldRotation(FName BoneName);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugIK(class UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogIKState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetIKStateDescription();

protected:
    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComp;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComp;

    // Internal state
    float TimeSinceLastUpdate;
    bool bIKSystemInitialized;
    
    // Cached bone indices for performance
    int32 LeftFootBoneIndex;
    int32 RightFootBoneIndex;
    int32 LeftHandBoneIndex;
    int32 RightHandBoneIndex;

    // Environmental state
    FVector LastGroundNormal;
    float LastSlopeAngle;
    float TerrainRoughness;

    // Performance optimization
    int32 FramesSinceLastUpdate;
    bool bNeedsUpdate;

private:
    // Internal helper functions
    void InitializeIKSystem();
    void CacheBoneIndices();
    bool IsValidBoneIndex(int32 BoneIndex) const;
    void UpdateIKTargetConfig(FIKTargetConfig& Config, float DeltaTime);
    FVector ClampIKTarget(const FVector& TargetLocation, const FVector& OriginalLocation, float MaxReach);
};

/**
 * Advanced Foot IK Component
 * Specialized component for complex foot placement on uneven terrain
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdvancedFootIK : public UProceduralIKSystem
{
    GENERATED_BODY()

public:
    UAdvancedFootIK();

    // Advanced foot placement settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Foot IK")
    bool bEnableFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Foot IK")
    bool bEnableFootPlanting; // Prevents foot sliding during steps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Foot IK")
    float FootPlantingThreshold; // Speed below which foot is considered planted

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Foot IK")
    bool bEnableStepPrediction; // Predicts where foot will land

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Foot IK")
    float StepPredictionDistance;

    // Terrain analysis
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Analysis")
    bool bAnalyzeTerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Analysis")
    float TerrainSampleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Analysis")
    int32 TerrainSampleCount;

    // Current advanced state
    UPROPERTY(BlueprintReadOnly, Category = "Advanced State")
    bool bLeftFootPlanted;

    UPROPERTY(BlueprintReadOnly, Category = "Advanced State")
    bool bRightFootPlanted;

    UPROPERTY(BlueprintReadOnly, Category = "Advanced State")
    FVector PredictedLeftFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Advanced State")
    FVector PredictedRightFootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Advanced State")
    float TerrainStability; // 0-1, how stable the current terrain is

    // Override base update function
    virtual void UpdateFootIK(float DeltaTime) override;

    // Advanced functions
    UFUNCTION(BlueprintCallable, Category = "Advanced Foot IK")
    void UpdateFootPlanting(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced Foot IK")
    void PredictFootLanding(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced Foot IK")
    void AnalyzeTerrainStability(const FVector& Location, float& OutStability);

    UFUNCTION(BlueprintCallable, Category = "Advanced Foot IK")
    bool ShouldPlantFoot(bool bLeftFoot, float CurrentSpeed);

    UFUNCTION(BlueprintCallable, Category = "Advanced Foot IK")
    FVector CalculateOptimalFootPlacement(const FVector& PredictedLocation, const FVector& CurrentLocation);

protected:
    // Advanced internal state
    FVector LeftFootPlantedLocation;
    FVector RightFootPlantedLocation;
    float LeftFootPlantTimer;
    float RightFootPlantTimer;
    
    // Terrain analysis cache
    TArray<FVector> TerrainSamples;
    TArray<float> TerrainHeights;
    float LastTerrainAnalysisTime;
};