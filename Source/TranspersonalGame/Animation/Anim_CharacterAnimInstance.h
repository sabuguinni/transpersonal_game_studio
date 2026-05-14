#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Attacking   UMETA(DisplayName = "Attacking")
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
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float JumpHeight = 0.0f;
};

/**
 * Animation Instance for TranspersonalCharacter
 * Handles movement states, combat animations, and survival animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation Blueprint Event Graph
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatState CurrentCombatState = EAnim_CombatState::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Character", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* CharacterMovement = nullptr;

    // Animation Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters", meta = (AllowPrivateAccess = "true"))
    float WalkSpeed = 150.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters", meta = (AllowPrivateAccess = "true"))
    float RunSpeed = 400.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Parameters", meta = (AllowPrivateAccess = "true"))
    float CrouchSpeed = 100.0f;

    // Animation Blend Values
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend", meta = (AllowPrivateAccess = "true"))
    float IdleToWalkBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend", meta = (AllowPrivateAccess = "true"))
    float WalkToRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend", meta = (AllowPrivateAccess = "true"))
    float DirectionalBlend = 0.0f;

    // State Transition Flags
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Transitions", meta = (AllowPrivateAccess = "true"))
    bool bShouldEnterJump = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Transitions", meta = (AllowPrivateAccess = "true"))
    bool bShouldEnterFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Transitions", meta = (AllowPrivateAccess = "true"))
    bool bShouldEnterLanding = false;

public:
    // Public Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation|Control")
    void TriggerBlockAnimation();

    // Getters for Animation Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation|Getters")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation|Getters")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Animation|Getters")
    FAnim_MovementData GetMovementData() const { return MovementData; }

private:
    // Internal State Update Functions
    void UpdateMovementData();
    void UpdateMovementState();
    void UpdateBlendValues();
    void UpdateTransitionFlags();

    // State Calculation Helpers
    EAnim_MovementState CalculateMovementState() const;
    float CalculateDirectionalBlend() const;
    float CalculateSpeedBlend() const;

    // Animation Event Handlers
    UFUNCTION()
    void OnJumpAnimationFinished();

    UFUNCTION()
    void OnAttackAnimationFinished();

    UFUNCTION()
    void OnLandingAnimationFinished();

    // Internal State Variables
    float LastGroundSpeed = 0.0f;
    float StateChangeTimer = 0.0f;
    bool bWasInAir = false;
    bool bJustLanded = false;
};