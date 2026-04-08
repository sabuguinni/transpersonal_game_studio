#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearch/PoseSearchDatabase.h"
#include "Animation/PoseSearch/PoseSearchSchema.h"
#include "IKRig.h"
#include "AnimationSystemCore.generated.h"

/**
 * Core Animation System for Transpersonal Game Studio
 * Handles Motion Matching, IK, and character-specific animation logic
 * 
 * Design Philosophy:
 * - Every movement tells a story about the character
 * - Weight and intention in every gesture
 * - Seamless adaptation to procedural terrain
 */

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Climbing,
    Swimming,
    Falling,
    Landing,
    Injured,
    Fearful,
    Cautious,
    Confident
};

UENUM(BlueprintType)
enum class ETerrainAdaptation : uint8
{
    Flat,
    Uphill,
    Downhill,
    Rocky,
    Muddy,
    Water,
    Vegetation
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    // Core personality traits affecting movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Confidence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Nervousness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fatigue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InjuryLevel = 0.0f;

    // Movement characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float BaseMovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float StepVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PosturalTension = 0.2f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Motion Matching Database Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void InitializeMotionMatchingDatabases();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    UPoseSearchDatabase* GetDatabaseForState(ECharacterMovementState State, ETerrainAdaptation Terrain);

    // Character Animation Profile Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCharacterAnimationProfile(const FCharacterAnimationProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FCharacterAnimationProfile GetCharacterAnimationProfile() const { return CurrentProfile; }

    // IK System Management
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTerrainAdaptation(const FVector& FootLocation, float TerrainHeight);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetIKTargets(const TArray<FVector>& FootTargets);

protected:
    // Motion Matching Databases
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TMap<ECharacterMovementState, UPoseSearchDatabase*> MovementDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    TMap<ETerrainAdaptation, UPoseSearchDatabase*> TerrainDatabases;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK System")
    UIKRigDefinition* PlayerIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK System")
    float MaxIKReach = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK System")
    float IKBlendSpeed = 5.0f;

    // Character Profile
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    FCharacterAnimationProfile CurrentProfile;

    // Terrain Analysis
    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    TArray<FVector> FootIKTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain")
    ETerrainAdaptation CurrentTerrainType;

private:
    void AnalyzeTerrainType(const FVector& Location);
    float CalculateMovementWeight(ECharacterMovementState State) const;
};