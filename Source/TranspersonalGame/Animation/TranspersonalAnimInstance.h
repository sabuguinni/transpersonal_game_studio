#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

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

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Standing    UMETA(DisplayName = "Standing"),
    Crouched    UMETA(DisplayName = "Crouched"),
    Prone       UMETA(DisplayName = "Prone")
};

/**
 * UTranspersonalAnimInstance
 * AnimInstance for the prehistoric survivor player character.
 * Drives locomotion blend spaces, IK foot placement, and survival state transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    // ── Survival Stats ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float Stamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float Fear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured;

    // ── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator LeftFootIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator RightFootIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    float IKAlpha;

    // ── Upper Body Overlay ────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|UpperBody")
    float AimPitch;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|UpperBody")
    float AimYaw;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|UpperBody")
    bool bIsAiming;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|UpperBody")
    bool bIsAttacking;

    // ── Lean ─────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Lean")
    float LeanAngle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Lean")
    float AccelerationMagnitude;

private:
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalStats();
    void UpdateAimOffset();
    void UpdateLean(float DeltaSeconds);

    FVector PreviousVelocity;
    float LeanAlpha;
};
