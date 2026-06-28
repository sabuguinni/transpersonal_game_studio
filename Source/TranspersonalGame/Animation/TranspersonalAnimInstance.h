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
 * Animation Instance for the Transpersonal prehistoric survivor character.
 * Drives locomotion blend spaces, IK foot placement, and state transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion State ───────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_StanceType StanceType;

    /** Ground speed (XY plane) used to drive blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    /** Strafe direction angle (-180 to 180) for directional blending */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    /** Vertical velocity for jump/fall blending */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity;

    // ─── State Booleans ─────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsMoving;

    // ─── IK Foot Placement ──────────────────────────────────────────────────

    /** Left foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Left foot IK alpha (0=disabled, 1=fully active) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float LeftFootIKAlpha;

    /** Right foot IK alpha (0=disabled, 1=fully active) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float RightFootIKAlpha;

    /** Pelvis offset Z for IK foot adjustment */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    // ─── Survival State ─────────────────────────────────────────────────────

    /** 0-1 fatigue level — affects animation speed and posture */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FatigueLevel;

    /** 0-1 fear level — affects movement jitter and breathing animation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    /** 0-1 injury level — drives limping blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float InjuryLevel;

    // ─── Aim Offsets ────────────────────────────────────────────────────────

    /** Pitch for aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    /** Yaw for aim offset (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

    // ─── Internal ───────────────────────────────────────────────────────────

private:
    /** Cached owning character */
    UPROPERTY()
    class ATranspersonalCharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Perform foot IK trace for the given socket name */
    void UpdateFootIK(const FName& FootSocketName, FVector& OutIKLocation, float& OutIKAlpha);

    /** Determine locomotion state from current movement */
    EAnim_LocomotionState DetermineLocomotionState() const;

    /** Interpolation speed for IK smoothing */
    static constexpr float IKInterpSpeed = 15.0f;

    /** Minimum speed to be considered "moving" */
    static constexpr float MovingSpeedThreshold = 10.0f;

    /** Foot IK trace distance below character */
    static constexpr float FootIKTraceDistance = 60.0f;
};
