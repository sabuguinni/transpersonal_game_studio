#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    FAnim_MovementData()
        : Speed(0.0f)
        , Direction(0.0f)
        , bIsInAir(false)
        , bIsCrouching(false)
        , bIsAccelerating(false)
        , MovementState(EAnim_MovementState::Idle)
    {
    }
};

/**
 * Core animation instance for TranspersonalCharacter
 * Handles movement state machine and animation blending
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwningCharacter;

    // Movement Component Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // Animation Sequences
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* WalkAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* RunAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    UAnimSequence* CrouchWalkAnimation;

    // Animation Blending Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float WalkRunBlendAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Blending", meta = (AllowPrivateAccess = "true"))
    float IdleToMovementBlendAlpha;

    // State Machine Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendingParameters();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterMovementState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterJumpState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterFallingState() const;

private:
    // Internal state tracking
    EAnim_MovementState PreviousMovementState;
    float StateTransitionTime;
    float MaxWalkSpeed;
    float MaxRunSpeed;

    // Helper functions
    void UpdateMovementData();
    void CalculateDirection();
    void UpdateBlendAlpha();
    EAnim_MovementState DetermineMovementState() const;
};