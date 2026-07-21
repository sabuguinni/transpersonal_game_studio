#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_PrimitiveAnimationController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Combat      UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    None            UMETA(DisplayName = "None"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bShouldMove = false;
};

/**
 * Primitive Animation Controller for prehistoric human characters
 * Handles basic locomotion, combat, and survival animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveAnimationController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveAnimationController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    // Current movement state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState CurrentMovementState;

    // Current combat state
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CurrentCombatState;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float WalkThreshold = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float RunThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float IdleThreshold = 5.0f;

    // Combat parameters
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking = false;

    // Survival state parameters
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

public:
    // Animation state functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalState();

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerBlockAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDodgeAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearReaction();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const { return MovementData.bShouldMove; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInCombat() const { return bIsInCombat; }

private:
    void UpdateMovementState();
    void UpdateAnimationParameters();
    void HandleStateTransitions();
};