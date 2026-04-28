// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
// FIXME: Missing header - #include "IKRig.h"
// DISABLED: #include "IKRigDataTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimationSystemCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnimationSystem, Log, All);

/**
 * Core animation system that handles Motion Matching, IK, and procedural animations
 * Based on RDR2's approach: every movement tells a story about the character
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAnimationSystemCore();

    // Core Animation State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    float StaminaLevel;

    // Motion Matching Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* CombatDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* DefaultSchema;

    // IK Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    class UIKRigDefinition* CharacterIKRig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float HandIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableHandIK;

    // Animation Databases
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<class UAnimSequence*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<class UAnimSequence*> WalkAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<class UAnimSequence*> RunAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<class UAnimSequence*> JumpAnimations;

    // Character-Specific Animation Traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float ConfidenceLevel; // 0-1: affects posture and gait

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float FatigueLevel; // 0-1: affects animation speed and precision

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float InjuryLevel; // 0-1: affects movement asymmetry

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float AgeModifier; // affects movement speed and stability

    // Environmental Adaptation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float TerrainRoughness; // affects foot placement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float SlopeAngle; // affects body lean

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bIsOnUnstableSurface;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingQuery(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UPoseSearchDatabase* SelectActiveDatabase();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void BlendBetweenDatabases(float BlendWeight);

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateFootIK(class USkeletalMeshComponent* SkeletalMesh, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateHandIK(class USkeletalMeshComponent* SkeletalMesh, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector CalculateFootPlacement(const FVector& FootLocation, const FVector& Normal);

    // Character Trait Functions
    UFUNCTION(BlueprintCallable, Category = "Character Traits")
    void ApplyCharacterTraits(float& AnimationSpeed, float& PostureOffset);

    UFUNCTION(BlueprintCallable, Category = "Character Traits")
    void UpdateFatigueLevel(float DeltaTime, float MovementIntensity);

    UFUNCTION(BlueprintCallable, Category = "Character Traits")
    void ApplyInjuryEffects(FVector& MovementVector);

    // Environmental Adaptation Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void AdaptToTerrain(const FVector& SurfaceNormal, float SurfaceRoughness);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void UpdateEnvironmentalFactors(const FVector& Location);

    // Animation Blending
    UFUNCTION(BlueprintCallable, Category = "Animation Blending")
    float CalculateBlendWeight(const FString& AnimationType, float CurrentState);

    UFUNCTION(BlueprintCallable, Category = "Animation Blending")
    void SmoothTransition(float& CurrentValue, float TargetValue, float DeltaTime, float BlendSpeed);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugAnimationState(class UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAnimationMetrics();

protected:
    // Internal state tracking
    float LastMovementSpeed;
    FVector LastMovementDirection;
    float BlendTimer;
    
    // Performance optimization
    float UpdateFrequency;
    float TimeSinceLastUpdate;

    // Animation quality settings
    int32 AnimationLOD;
    bool bHighQualityMode;
};

/**
 * Character-specific animation instance that integrates with the core system
 * Each character type (protagonist, NPCs) can have specialized behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UCharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Core animation system reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation System")
    class UAnimationSystemCore* AnimationSystem;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Animation state variables for Blueprint access
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float LeanAmount;

    // Motion Matching state
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchDatabase* CurrentDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    float DatabaseBlendWeight;

    // IK state
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    // Character traits
    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    float ConfidenceModifier;

    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    float FatigueModifier;

    UPROPERTY(BlueprintReadOnly, Category = "Character Traits")
    float InjuryModifier;

protected:
    // Update functions called from NativeUpdateAnimation
    void UpdateMovementValues();
    void UpdateMotionMatching(float DeltaTime);
    void UpdateIKValues(float DeltaTime);
    void UpdateCharacterTraits(float DeltaTime);
};

/**
 * Specialized animation instance for the protagonist paleontologist
 * Includes scientific observation behaviors and stress responses
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UProtagonistAnimInstance : public UCharacterAnimInstance
{
    GENERATED_BODY()

public:
    UProtagonistAnimInstance();

    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Protagonist-specific states
    UPROPERTY(BlueprintReadOnly, Category = "Protagonist")
    bool bIsObservingDinosaur;

    UPROPERTY(BlueprintReadOnly, Category = "Protagonist")
    float StressLevel; // 0-1: affects breathing, fidgeting

    UPROPERTY(BlueprintReadOnly, Category = "Protagonist")
    float CuriosityLevel; // affects head movements, posture

    UPROPERTY(BlueprintReadOnly, Category = "Protagonist")
    bool bIsHoldingTool;

    UPROPERTY(BlueprintReadOnly, Category = "Protagonist")
    bool bIsCrafting;

    // Scientific behavior animations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Behavior")
    class UAnimMontage* ObservationMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Behavior")
    class UAnimMontage* NoteTakingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scientific Behavior")
    class UAnimMontage* ToolUseMontage;

protected:
    void UpdateProtagonistBehaviors(float DeltaTime);
    void UpdateStressResponse(float DeltaTime);
    void UpdateScientificBehaviors(float DeltaTime);
};