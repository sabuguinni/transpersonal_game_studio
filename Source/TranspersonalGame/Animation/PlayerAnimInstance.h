#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerAnimInstance.generated.h"

/**
 * Animation instance for the prehistoric human player character.
 * Drives idle/walk/run/sprint/jump/crouch/attack blend spaces
 * and foot IK via Motion Warping.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ─────────────────────────────────────────────────────────

    /** Current ground speed (cm/s). Drives walk/run blend space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Speed;

    /** Lateral direction relative to character facing (-180..180). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Direction;

    /** Smoothed speed used for blend space to avoid jitter. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float SmoothedSpeed;

    /** True when the character is airborne. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    /** True when the character is sprinting (speed > SprintThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    /** True when the character is accelerating (input applied). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    // ── Combat / Actions ────────────────────────────────────────────────────

    /** True when the character is in combat stance. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInCombat;

    /** True when the character is aiming a ranged weapon. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAiming;

    /** True when a melee attack montage is playing. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    // ── Survival Stats ──────────────────────────────────────────────────────

    /** Normalised exhaustion (0=fresh, 1=exhausted). Affects posture. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
              meta = (AllowPrivateAccess = "true"))
    float ExhaustionAlpha;

    /** Normalised injury (0=healthy, 1=critical). Affects limp blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
              meta = (AllowPrivateAccess = "true"))
    float InjuryAlpha;

    /** Fear level (0=calm, 1=panicked). Affects idle fidget rate. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival",
              meta = (AllowPrivateAccess = "true"))
    float FearAlpha;

    // ── Foot IK ─────────────────────────────────────────────────────────────

    /** Left foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation;

    /** Alpha blend for foot IK (0=off, 1=full IK). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK",
              meta = (AllowPrivateAccess = "true"))
    float FootIKAlpha;

    // ── Tuning ──────────────────────────────────────────────────────────────

    /** Speed above which the character is considered sprinting. */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Tuning")
    float SprintThreshold;

    /** Speed smoothing interpolation rate. */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Tuning")
    float SpeedSmoothingRate;

    /** Trace distance below foot for IK ground detection. */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Tuning")
    float FootIKTraceDistance;

private:
    /** Cached owning character. */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component. */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Perform a line trace below the given bone to find the ground. */
    FVector TraceFootIKLocation(FName BoneName) const;

    /** Compute direction angle between velocity and actor forward. */
    float ComputeDirection(const FVector& Velocity, const FRotator& ActorRotation) const;
};
