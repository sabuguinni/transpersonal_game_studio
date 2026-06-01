#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_TribalCharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crafting    UMETA(DisplayName = "Crafting"),
    SpearReady  UMETA(DisplayName = "Spear Ready"),
    Throwing    UMETA(DisplayName = "Throwing")
};

UENUM(BlueprintType)
enum class EAnim_TribalActionState : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Fleeing         UMETA(DisplayName = "Fleeing")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float EnergyLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
    bool bHasSpear = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
    bool bHasTool = false;

    FAnim_TribalAnimationData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_TribalMovementState::Idle;
        ActionState = EAnim_TribalActionState::None;
        FearLevel = 0.0f;
        EnergyLevel = 1.0f;
        bHasSpear = false;
        bHasTool = false;
    }
};

/**
 * Animation Instance for tribal characters in the prehistoric survival game.
 * Handles movement states, survival actions, and tool usage animations.
 * Integrates with Motion Matching for fluid character movement.
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_TribalCharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalCharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnim_TribalAnimationData AnimData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    // Movement Component Reference
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    class UCharacterMovementComponent* MovementComponent;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Idle")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Movement")
    class UBlendSpace* WalkRunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Movement")
    class UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Movement")
    class UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Movement")
    class UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Actions")
    class UAnimMontage* SpearThrowMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Actions")
    class UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Actions")
    class UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations|Reactions")
    class UAnimMontage* FearReactionMontage;

    // Animation Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float MovementSpeedThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Control")
    float AnimationBlendSpeed = 5.0f;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootIKTraceDistance = 50.0f;

public:
    // Animation State Control
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_TribalMovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetActionState(EAnim_TribalActionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayActionMontage(EAnim_TribalActionState ActionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void StopActionMontage();

    // Survival Animation Triggers
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TriggerFearReaction(float FearIntensity);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateEnergyLevel(float NewEnergyLevel);

    // Tool Animation Control
    UFUNCTION(BlueprintCallable, Category = "Tools")
    void SetHasSpear(bool bNewHasSpear);

    UFUNCTION(BlueprintCallable, Category = "Tools")
    void SetHasTool(bool bNewHasTool);

    UFUNCTION(BlueprintCallable, Category = "Tools")
    void PlaySpearThrowAnimation();

    UFUNCTION(BlueprintCallable, Category = "Tools")
    void PlayCraftingAnimation();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation Data")
    FAnim_TribalAnimationData GetAnimationData() const { return AnimData; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_TribalMovementState GetMovementState() const { return AnimData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation Data")
    EAnim_TribalActionState GetActionState() const { return AnimData.ActionState; }

protected:
    // Internal Update Functions
    void UpdateMovementData();
    void UpdateMovementState();
    void UpdateActionState();
    void UpdateSurvivalData();
    void UpdateToolData();

    // Helper Functions
    bool IsMoving() const;
    bool IsRunning() const;
    bool IsInAir() const;
    float CalculateDirection() const;

private:
    // Internal State
    float LastUpdateTime = 0.0f;
    bool bWasInAir = false;
    EAnim_TribalMovementState PreviousMovementState = EAnim_TribalMovementState::Idle;
    EAnim_TribalActionState PreviousActionState = EAnim_TribalActionState::None;
};