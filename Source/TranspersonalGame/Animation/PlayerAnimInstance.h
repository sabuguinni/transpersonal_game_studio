#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

/**
 * UPlayerAnimInstance
 * Animation Blueprint logic for the prehistoric human player character.
 * Drives locomotion blend space (idle/walk/run/sprint), jump, crouch,
 * attack, and foot IK states.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Current ground speed (cm/s). Drives the locomotion blend space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Smoothed speed for blend-space damping (avoids jitter). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    /** Lateral movement direction (-180 to 180) relative to actor forward. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when the character is airborne. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when the character is sprinting (speed > SprintThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** True when the character is moving (speed > MovingThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    // ── Survival States ─────────────────────────────────────────────────────

    /** True when character is sneaking (low crouch + slow speed). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsSneaking;

    /** True when character is climbing. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsClimbing;

    /** Fear level 0-1. Affects posture and movement animations. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    /** Stamina 0-1. Low stamina triggers exhaustion blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaLevel;

    // ── Combat ──────────────────────────────────────────────────────────────

    /** True when character is in combat stance. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsInCombat;

    /** True when attack montage is playing. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** Upper body aim pitch (-90 to 90) for aim offset. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float AimPitch;

    /** Upper body aim yaw (-90 to 90) for aim offset. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float AimYaw;

    // ── Foot IK ─────────────────────────────────────────────────────────────

    /** Left foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKLocation;

    /** Pelvis offset Z to keep body level on uneven terrain. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float PelvisOffset;

    /** True when foot IK is active (character on ground). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    bool bFootIKEnabled;

    // ── Thresholds (tweakable in BP) ────────────────────────────────────────

    /** Speed above which bIsMoving becomes true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float MovingThreshold;

    /** Speed above which bIsSprinting becomes true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float SprintThreshold;

    /** Speed smoothing interpolation rate. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float SpeedSmoothingRate;

    /** Foot IK trace distance below foot bone. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Config")
    float FootIKTraceDistance;

private:
    /** Cached owner character. */
    UPROPERTY()
    ACharacter* OwnerCharacter;

    /** Cached movement component. */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    /** Performs a foot IK line trace and returns the adjusted location. */
    FVector TraceFootIK(FName SocketName, float& OutPelvisDelta) const;

    /** Updates foot IK for both feet and computes pelvis offset. */
    void UpdateFootIK();
};
