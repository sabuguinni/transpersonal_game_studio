#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal (prehistoric survivor) character.
 * Drives locomotion blend space, jump/fall states, foot IK, and look-at IK.
 * Designed for Motion Matching readiness — all state variables exposed as UPROPERTY.
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

    /** Current ground speed (cm/s). Drives the locomotion blend space axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed = 0.f;

    /** True when the character has lateral/forward velocity above threshold. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving = false;

    /** True when the character is sprinting (speed > SprintThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting = false;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching = false;

    /** Movement direction angle relative to character facing (-180..180). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float MovementDirection = 0.f;

    // ── Airborne ─────────────────────────────────────────────────────────────

    /** True when the character is in the air (jumping or falling). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    bool bIsInAir = false;

    /** Vertical velocity (positive = rising, negative = falling). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    float VerticalVelocity = 0.f;

    /** True during the first frames of a jump (rising phase). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    bool bIsJumping = false;

    /** True when falling (vertical velocity < FallThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    bool bIsFalling = false;

    // ── Survival State ────────────────────────────────────────────────────────

    /** Stamina 0-1. Below 0.2 triggers exhaustion blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized = 1.f;

    /** Fear 0-1. Above 0.7 triggers fear tremor additive layer. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearNormalized = 0.f;

    /** True when character is wounded (health < 0.3). Drives limping blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsWounded = false;

    // ── Foot IK ──────────────────────────────────────────────────────────────

    /** Left foot IK target offset in component space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKOffset = FVector::ZeroVector;

    /** Right foot IK target offset in component space. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKOffset = FVector::ZeroVector;

    /** Pelvis adjustment to keep body centred between foot IK targets. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float PelvisOffset = 0.f;

    /** Alpha for foot IK blending (0 = off, 1 = full IK). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float FootIKAlpha = 1.f;

    // ── Look-At IK ───────────────────────────────────────────────────────────

    /** World-space look-at target (e.g. nearest threat or point of interest). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|LookAt")
    FVector LookAtTarget = FVector::ZeroVector;

    /** Alpha for look-at IK (0 = off, 1 = full head tracking). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|LookAt")
    float LookAtAlpha = 0.f;

    // ── Thresholds (configurable per character) ───────────────────────────────

    /** Speed above which bIsMoving becomes true (cm/s). */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float MoveThreshold = 10.f;

    /** Speed above which bIsSprinting becomes true (cm/s). */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float SprintThreshold = 375.f;

    /** Vertical velocity below which bIsFalling becomes true (cm/s). */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float FallThreshold = -200.f;

    /** Trace half-height for foot IK ground detection (cm). */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float FootIKTraceDistance = 50.f;

protected:
    /** Updates locomotion variables from the owning character's movement component. */
    void UpdateLocomotion(float DeltaSeconds);

    /** Updates airborne variables (jump/fall state). */
    void UpdateAirborne(float DeltaSeconds);

    /** Updates survival state variables from the owning character's stats. */
    void UpdateSurvivalState(float DeltaSeconds);

    /** Performs two line traces to compute foot IK offsets. */
    void UpdateFootIK(float DeltaSeconds);

    /** Updates look-at target based on nearest perceived threat. */
    void UpdateLookAt(float DeltaSeconds);

private:
    /** Cached owning pawn (set in NativeInitializeAnimation). */
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    /** Cached movement component. */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComp = nullptr;

    /** Smooth foot IK offset interpolation speed. */
    static constexpr float FootIKInterpSpeed = 15.f;

    /** Smooth look-at alpha interpolation speed. */
    static constexpr float LookAtInterpSpeed = 5.f;
};
