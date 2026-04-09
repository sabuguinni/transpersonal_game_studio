#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "AdaptiveIKSystem.generated.h"

class UIKRigDefinition;
class UIKRigProcessor;

/**
 * IK Target data for a single limb (foot, hand, etc.)
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FIKTarget
{
    GENERATED_BODY()

    // Target bone name (e.g., "foot_l", "foot_r")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target")
    FName BoneName;

    // Current target location in world space
    UPROPERTY(BlueprintReadOnly, Category = "IK Target")
    FVector TargetLocation;

    // Current target rotation in world space
    UPROPERTY(BlueprintReadOnly, Category = "IK Target")
    FRotator TargetRotation;

    // Original bone location (for reference)
    UPROPERTY(BlueprintReadOnly, Category = "IK Target")
    FVector OriginalLocation;

    // IK influence weight (0.0 = no IK, 1.0 = full IK)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Target", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKWeight;

    // Whether this target is currently active
    UPROPERTY(BlueprintReadOnly, Category = "IK Target")
    bool bIsActive;

    // Distance from original position (for debugging)
    UPROPERTY(BlueprintReadOnly, Category = "IK Target")
    float DistanceFromOriginal;

    FIKTarget()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        OriginalLocation = FVector::ZeroVector;
        IKWeight = 1.0f;
        bIsActive = false;
        DistanceFromOriginal = 0.0f;
    }
};

/**
 * Configuration for terrain adaptation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainAdaptationConfig
{
    GENERATED_BODY()

    // Maximum distance to trace for ground detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation", meta = (ClampMin = "10.0", ClampMax = "500.0"))
    float MaxTraceDistance;

    // How far ahead to look for terrain changes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation", meta = (ClampMin = "10.0", ClampMax = "200.0"))
    float PredictionDistance;

    // Maximum height difference to adapt to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation", meta = (ClampMin = "5.0", ClampMax = "100.0"))
    float MaxAdaptationHeight;

    // Speed of IK adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation", meta = (ClampMin = "1.0", ClampMax = "20.0"))
    float AdaptationSpeed;

    // Minimum slope angle to trigger adaptation (degrees)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation", meta = (ClampMin = "0.0", ClampMax = "45.0"))
    float MinSlopeAngle;

    // Collision channel for ground detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    TEnumAsByte<ECollisionChannel> GroundTraceChannel;

    FTerrainAdaptationConfig()
    {
        MaxTraceDistance = 150.0f;
        PredictionDistance = 50.0f;
        MaxAdaptationHeight = 50.0f;
        AdaptationSpeed = 8.0f;
        MinSlopeAngle = 5.0f;
        GroundTraceChannel = ECC_WorldStatic;
    }
};

/**
 * Adaptive IK System Component
 * Handles real-time IK adaptation for character feet and hands
 * Provides smooth terrain adaptation and natural movement on uneven surfaces
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdaptiveIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAdaptiveIKSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetTerrainAdaptationConfig(const FTerrainAdaptationConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Adaptive IK")
    FTerrainAdaptationConfig GetTerrainAdaptationConfig() const { return TerrainConfig; }

    // IK Target management
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void AddIKTarget(const FName& BoneName, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void RemoveIKTarget(const FName& BoneName);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetIKTargetWeight(const FName& BoneName, float Weight);

    UFUNCTION(BlueprintPure, Category = "Adaptive IK")
    FIKTarget GetIKTarget(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Adaptive IK")
    TArray<FIKTarget> GetAllIKTargets() const;

    // Terrain adaptation
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void EnableTerrainAdaptation(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Adaptive IK")
    bool IsTerrainAdaptationEnabled() const { return bTerrainAdaptationEnabled; }

    // Manual IK control
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetManualIKTarget(const FName& BoneName, const FVector& WorldLocation, const FRotator& WorldRotation, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void ClearManualIKTarget(const FName& BoneName);

    // Ground detection
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    float GetGroundHeight(const FVector& Location) const;

    // Character state integration
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetCharacterMovementState(bool bIsMoving, float MovementSpeed);

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetCharacterStance(const FString& StanceName);

    // Animation Blueprint integration
    UFUNCTION(BlueprintPure, Category = "Animation Integration")
    FVector GetIKTargetLocation(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Animation Integration")
    FRotator GetIKTargetRotation(const FName& BoneName) const;

    UFUNCTION(BlueprintPure, Category = "Animation Integration")
    float GetIKTargetWeight(const FName& BoneName) const;

    // Debugging
    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void DebugDrawIKTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Adaptive IK")
    void SetDebugMode(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Adaptive IK")
    FString GetDebugString() const;

protected:
    // Core configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FTerrainAdaptationConfig TerrainConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bTerrainAdaptationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bDebugMode;

    // IK Targets
    UPROPERTY(BlueprintReadOnly, Category = "IK Targets")
    TArray<FIKTarget> IKTargets;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Character state
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsCharacterMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    float CurrentMovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    FString CurrentStance;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    float LastUpdateTime;

    // Smoothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
    float IKSmoothingSpeed;

    TMap<FName, FVector> PreviousTargetLocations;
    TMap<FName, FRotator> PreviousTargetRotations;

private:
    // Core IK processing
    void UpdateIKTargets(float DeltaTime);
    void ProcessTerrainAdaptation(FIKTarget& Target, float DeltaTime);
    void SmoothIKTransitions(FIKTarget& Target, float DeltaTime);

    // Ground detection
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    FRotator CalculateGroundRotation(const FVector& GroundNormal) const;

    // Utility functions
    FIKTarget* FindIKTarget(const FName& BoneName);
    const FIKTarget* FindIKTarget(const FName& BoneName) const;
    bool ShouldUpdateThisFrame() const;
    void CacheComponentReferences();

    // Prediction and smoothing
    FVector PredictFootPlacement(const FIKTarget& Target) const;
    float CalculateAdaptationWeight(const FIKTarget& Target) const;
};

/**
 * Animation Blueprint node data structure for IK integration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAdaptiveIKData
{
    GENERATED_BODY()

    // Left foot IK
    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    float LeftFootWeight;

    // Right foot IK
    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    float RightFootWeight;

    // Hip adjustment
    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    FVector HipOffset;

    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    float HipOffsetWeight;

    // Overall IK influence
    UPROPERTY(BlueprintReadWrite, Category = "IK Data")
    float GlobalIKWeight;

    FAdaptiveIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        LeftFootWeight = 0.0f;
        
        RightFootLocation = FVector::ZeroVector;
        RightFootRotation = FRotator::ZeroRotator;
        RightFootWeight = 0.0f;
        
        HipOffset = FVector::ZeroVector;
        HipOffsetWeight = 0.0f;
        
        GlobalIKWeight = 1.0f;
    }
};