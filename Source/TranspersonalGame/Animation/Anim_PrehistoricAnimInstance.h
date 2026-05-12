#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Anim_CharacterMovementController.h"
#include "Anim_PrehistoricAnimInstance.generated.h"

/**
 * Custom Animation Instance for prehistoric survival characters
 * Handles locomotion, survival states, and contextual animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Movement data from controller
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    // Cached movement values for animation
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSwimming;

    // Survival state flags
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsFearful;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHungry;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsThirsty;

    // Survival values
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    // Animation state machine states
    UPROPERTY(BlueprintReadOnly, Category = "State Machine")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "State Machine")
    EAnim_SurvivalState CurrentSurvivalState;

    // Transition conditions
    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bCanTransitionToIdle;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bCanTransitionToWalk;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bCanTransitionToRun;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bCanTransitionToJump;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bCanTransitionToCrouch;

    // Contextual animation triggers
    UPROPERTY(BlueprintReadOnly, Category = "Contextual")
    bool bShouldPlayFearReaction;

    UPROPERTY(BlueprintReadOnly, Category = "Contextual")
    bool bShouldPlayInjuryReaction;

    UPROPERTY(BlueprintReadOnly, Category = "Contextual")
    bool bShouldPlayExhaustionIdle;

    // Animation blending weights
    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float LocomotionWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float SurvivalWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float FearWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float InjuryWeight;

    // Animation event handlers
    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnLanded();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnFearTriggered(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnInjuryReceived();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnStaminaDepleted();

protected:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UAnim_CharacterMovementController* MovementController;

    UPROPERTY()
    class UCharacterMovementComponent* CharacterMovement;

    // Internal state tracking
    UPROPERTY()
    EAnim_MovementState PreviousMovementState;

    UPROPERTY()
    EAnim_SurvivalState PreviousSurvivalState;

    UPROPERTY()
    float StateChangeTimer;

    UPROPERTY()
    float MinStateTime;

    // Animation update methods
    void UpdateMovementData();
    void UpdateSurvivalFlags();
    void UpdateTransitionConditions();
    void UpdateBlendingWeights();
    void UpdateContextualTriggers();

    // State transition helpers
    bool CanTransitionFromState(EAnim_MovementState FromState, EAnim_MovementState ToState);
    bool HasBeenInStateForMinTime();
    void OnMovementStateChanged(EAnim_MovementState NewState);
    void OnSurvivalStateChanged(EAnim_SurvivalState NewState);

    // Animation curve helpers
    UFUNCTION(BlueprintCallable, Category = "Animation Curves")
    float GetAnimationCurveValue(FName CurveName);

    UFUNCTION(BlueprintCallable, Category = "Animation Curves")
    void SetAnimationCurveValue(FName CurveName, float Value);

private:
    // Animation smoothing
    float SmoothSpeed;
    float SmoothDirection;
    float SpeedInterpRate;
    float DirectionInterpRate;
    
    // State timing
    float TimeSinceLastStateChange;
    
    // Event flags (reset each frame)
    bool bJumpEventTriggered;
    bool bLandEventTriggered;
    bool bFearEventTriggered;
    bool bInjuryEventTriggered;
};