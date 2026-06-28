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
 * Animation instance for the prehistoric survivor player character.
 * Drives locomotion blend spaces, IK foot placement, and combat states.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    /** Ground speed (XY plane only), used to drive blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Strafe/direction angle (-180 to 180), used for 2D blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** Vertical velocity — positive = rising, negative = falling */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    // ── State Booleans ────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    bool bIsAiming;

    // ── Survival Stats (drive animation intensity) ────────────────────────
    /** 0-1: low stamina causes laboured breathing animation */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized;

    /** 0-1: low health causes limping / hunched posture */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthNormalized;

    /** 0-1: high fear causes trembling additive layer */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearNormalized;

    // ── IK Foot Placement ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float RightFootIKAlpha;

    /** Pelvis offset to keep body centred between feet on uneven terrain */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float PelvisOffset;

    // ── Lean / Additive ───────────────────────────────────────────────────
    /** Lateral lean amount for banking into turns */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Additive")
    float LeanAmount;

    /** Aim pitch for upper-body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Additive")
    float AimPitch;

    /** Aim yaw for upper-body aim offset (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Additive")
    float AimYaw;

    // ── Thresholds ────────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float SprintSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float IKTraceDistance;

protected:
    /** Cached reference to owning character */
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;

private:
    void UpdateLocomotionState();
    void UpdateIKFootPlacement();
    void UpdateSurvivalAnimations();
    void UpdateAimOffset();

    /** Smooth lean using exponential decay */
    float SmoothLean(float Current, float Target, float DeltaSeconds, float SmoothSpeed = 8.0f);

    float PreviousSpeed;
    float LeanSmoothed;
};
