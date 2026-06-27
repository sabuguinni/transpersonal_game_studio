// TranspersonalAnimInstance.h
// Agent #10 — Animation Agent
// Core Animation Instance for the prehistoric survivor character.
// Drives locomotion blend space, jump/fall states, and terrain-adaptive foot IK.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Locomotion state enum — drives state machine transitions
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    // ── State Flags ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsIdle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsCarrying;

    // ── Survival Stats (affect animation weight/posture) ────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    // ── Foot IK ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float FootIKLeftAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float FootIKRightAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootEffectorLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootEffectorLocation;

    // ── Setters (called from Character) ─────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetStaminaRatio(float InRatio);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFearLevel(float InFear);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetIsAttacking(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetIsCarrying(bool bCarry);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetIsDead(bool bDead);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    void UpdateLocomotionValues(float DeltaSeconds);
    void UpdateAirborneValues();
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
};
