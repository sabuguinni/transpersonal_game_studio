#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_TribalCharacterShowcase.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Combat      UMETA(DisplayName = "Combat"),
    Crafting    UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class EAnim_TribalActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    SpearThrow      UMETA(DisplayName = "Spear Throw"),
    ToolCrafting    UMETA(DisplayName = "Tool Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalAnimationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
    EAnim_TribalMovementState MovementState = EAnim_TribalMovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States")
    EAnim_TribalActionState ActionState = EAnim_TribalActionState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthLevel = 100.0f;

    FAnim_TribalAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_TribalMovementState::Idle;
        ActionState = EAnim_TribalActionState::None;
        bIsInCombat = false;
        StaminaLevel = 100.0f;
        HealthLevel = 100.0f;
    }
};

/**
 * Tribal Character Animation Showcase - Demonstrates prehistoric human movement and survival animations
 * Focuses on realistic tribal behavior: cautious movement, environmental awareness, survival actions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TribalCharacterShowcase : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterShowcase();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    FAnim_TribalAnimationData AnimData;

    // Character Reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    ACharacter* OwningCharacter;

    // Movement Component Reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    UCharacterMovementComponent* MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* TribalLocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* SpearThrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* ToolCraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* ClimbingMontage;

    // Animation Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float AnimationBlendSpeed = 5.0f;

    // Tribal Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float CautiousMovementMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float SurvivalStanceIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    bool bShowTribalCharacteristics = true;

public:
    // Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void PlayTribalAction(EAnim_TribalActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void StopTribalAction();

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetMovementState(EAnim_TribalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatMode(bool bInCombat);

    // Animation State Queries
    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsPlayingTribalAction() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_TribalMovementState GetCurrentMovementState() const;

    UFUNCTION(BlueprintPure, Category = "Animation State")
    EAnim_TribalActionState GetCurrentActionState() const;

    // Tribal Animation Showcase Functions
    UFUNCTION(BlueprintCallable, Category = "Showcase")
    void DemonstrateTribalMovement();

    UFUNCTION(BlueprintCallable, Category = "Showcase")
    void DemonstrateSurvivalActions();

    UFUNCTION(BlueprintCallable, Category = "Showcase")
    void DemonstrateHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Showcase")
    void ResetToIdleState();

protected:
    // Internal Update Functions
    void UpdateMovementData();
    void UpdateAnimationStates();
    void UpdateTribalBehavior();
    void BlendAnimationStates(float DeltaTime);

    // Tribal Animation Logic
    void ProcessTribalMovement();
    void ProcessSurvivalStance();
    void ProcessEnvironmentalAwareness();

    // Animation Transition Helpers
    bool CanTransitionToState(EAnim_TribalMovementState NewState) const;
    void OnMovementStateChanged(EAnim_TribalMovementState OldState, EAnim_TribalMovementState NewState);
    void OnActionStateChanged(EAnim_TribalActionState OldState, EAnim_TribalActionState NewState);

private:
    // Internal State Tracking
    EAnim_TribalMovementState PreviousMovementState;
    EAnim_TribalActionState PreviousActionState;
    float StateTransitionTimer;
    bool bIsTransitioning;

    // Animation Blending
    float CurrentBlendWeight;
    float TargetBlendWeight;
    
    // Tribal Behavior Timers
    float CautiousMovementTimer;
    float EnvironmentalCheckTimer;
    float SurvivalStanceTimer;
};