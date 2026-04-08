#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
    Idle,
    Walking,
    Running,
    Crouching,
    Sneaking,
    Climbing,
    Swimming,
    Falling,
    Landing,
    Dying,
    Interacting
};

UENUM(BlueprintType)
enum class ETerrainAdaptationLevel : uint8
{
    None,
    Light,      // Small rocks, slight inclines
    Medium,     // Uneven ground, roots
    Heavy,      // Steep terrain, large obstacles
    Extreme     // Cliff faces, very irregular surfaces
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Alert,
    Fearful,
    Panicked,
    Exhausted,
    Injured,
    Confident,
    Aggressive
};

/**
 * Central manager for all animation systems in the Jurassic survival game
 * Coordinates Motion Matching, IK systems, and emotional state animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<ECharacterMovementState, TObjectPtr<UPoseSearchDatabase>> MovementDatabases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> PlayerLocomotionSchema;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TObjectPtr<UPoseSearchSchema> DinosaurLocomotionSchema;

    // IK System Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKRange = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableHandIK = false; // For climbing

    // Emotional Animation System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional States")
    TMap<EEmotionalState, float> EmotionalBlendWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional States")
    float EmotionalTransitionSpeed = 2.0f;

    // Terrain Adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    ETerrainAdaptationLevel CurrentTerrainLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Adaptation")
    float TerrainAdaptationStrength = 1.0f;

    // Animation Quality Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousAnimations = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceThreshold = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAnimationLOD = true;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetCharacterMovementState(ECharacterMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetEmotionalState(EEmotionalState NewState, float BlendTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateTerrainAdaptation(ETerrainAdaptationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    UPoseSearchDatabase* GetCurrentMovementDatabase() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float GetEmotionalStateWeight(EEmotionalState State) const;

    UFUNCTION(BlueprintPure, Category = "Animation System")
    bool IsInFearState() const;

    UFUNCTION(BlueprintPure, Category = "Animation System")
    float GetMovementIntensity() const;

private:
    // Internal state tracking
    ECharacterMovementState CurrentMovementState;
    EEmotionalState CurrentEmotionalState;
    EEmotionalState TargetEmotionalState;
    
    float EmotionalBlendTimer;
    float MovementIntensity;
    
    // Performance tracking
    int32 ActiveAnimationCount;
    TArray<AActor*> NearbyAnimatedActors;

    // Internal functions
    void UpdateEmotionalBlending(float DeltaTime);
    void UpdatePerformanceMetrics();
    void OptimizeAnimationLOD();
};