#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "PrehistoricAnimInstance.generated.h"

// Forward declarations
class ACharacter;
class UPrimitiveAnimationController;

/**
 * Prehistoric Animation Instance
 * Main animation blueprint class for prehistoric characters
 * Handles state machine logic and animation blending for survival gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    TObjectPtr<ACharacter> OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    TObjectPtr<UCharacterMovementComponent> CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    TObjectPtr<UPrimitiveAnimationController> AnimationController;

    // Animation state variables (exposed to Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_BlendParameters BlendParams;

    // Movement variables
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleBreakChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleBreakMinInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float LookAroundChance = 0.05f;

    // Survival animation states
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsResting;

    // Combat animation states
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsDodging;

private:
    // Internal state tracking
    float LastIdleBreakTime;
    float LastLookAroundTime;
    bool bWasMoving;

    // Helper functions
    void UpdateMovementVariables();
    void UpdateSurvivalStates();
    void UpdateCombatStates();
    void CheckIdleAnimations();
    bool ShouldPlayIdleBreak() const;
    bool ShouldLookAround() const;

public:
    // Animation event functions (callable from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnLanded();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnCombatStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnCombatEnd();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnSurvivalActionStart(ESurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnSurvivalActionEnd();

    // Getters for Blueprint access
    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsIdleState() const { return MovementState == EAnim_MovementState::Idle; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsWalkingState() const { return MovementState == EAnim_MovementState::Walking; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsRunningState() const { return MovementState == EAnim_MovementState::Running; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsJumpingState() const { return MovementState == EAnim_MovementState::Jumping; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsFallingState() const { return MovementState == EAnim_MovementState::Falling; }

    UFUNCTION(BlueprintPure, Category = "Animation State")
    bool IsCrouchingState() const { return MovementState == EAnim_MovementState::Crouching; }
};