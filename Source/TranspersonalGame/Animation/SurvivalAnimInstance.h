#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "SurvivalAnimInstance.generated.h"

/**
 * Animation Blueprint instance for primitive survival character
 * Handles state machine and blend space logic
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USurvivalAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Animation state variables (exposed to Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FAnim_BlendData BlendData;

    // Movement parameters for blend spaces
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    // Aim offset parameters
    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    float AimYaw = 0.0f;

    // Character state
    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Character State")
    float StaminaPercentage = 1.0f;

    // Animation assets references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* IdleBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* WalkRunBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpStartSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpLoopSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpEndSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchIdleSequence = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchWalkSequence = nullptr;

    // Animation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float MovingThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    float InterpolationSpeed = 10.0f;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterState(float Health, float Stamina, bool bAlive, bool bInjured);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerHitReaction(const FVector& HitDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDeathAnimation();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterMovementState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterJumpState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterFallState() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UPrimitiveAnimationController* AnimController = nullptr;

    // Internal state
    float LastUpdateTime = 0.0f;
    bool bWasInAir = false;
    EAnim_MovementState PreviousState = EAnim_MovementState::Idle;

    // Helper functions
    void UpdateMovementData();
    void UpdateAnimationController();
    void HandleStateTransitions();
    void UpdateBlendSpaceParameters();
};