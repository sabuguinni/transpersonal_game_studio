// DinoSurvivorAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260623_005
// UAnimInstance subclass driving the survivor character state machine.
// States: Idle, Walk, Run, Crouch, Jump, Fall, Land, Attack, Death.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ─── Movement State Enum ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim"),
    Death       UMETA(DisplayName = "Death")
};

// ─── Combat State Enum ────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Torch       UMETA(DisplayName = "Torch")
};

// ─── Foot IK Data ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float PelvisOffset = 0.0f;
};

// ─── Lean Data ────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Lean")
    float LeanLeftRight = 0.0f;
};

// ─── Main AnimInstance Class ──────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ── Locomotion Properties ─────────────────────────────────────────────────

    /** Current ground speed (cm/s) — drives blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float GroundSpeed = 0.0f;

    /** Lateral speed for strafing blend */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float LateralSpeed = 0.0f;

    /** Direction of movement relative to character facing (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float MovementDirection = 0.0f;

    /** Vertical velocity — positive = rising, negative = falling */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float VerticalVelocity = 0.0f;

    /** Is the character currently in the air? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir = false;

    /** Is the character accelerating? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsAccelerating = false;

    /** Is the character crouching? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching = false;

    /** Is the character sprinting? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSprinting = false;

    /** Is the character swimming? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSwimming = false;

    /** Is the character climbing? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsClimbing = false;

    /** Is the character dead? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsDead = false;

    /** Current movement state enum */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    /** Current combat/weapon state */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_CombatState CombatState = EAnim_CombatState::Unarmed;

    // ── Survival Stats (drive animation intensity) ────────────────────────────

    /** Stamina 0-100 — affects run animation quality */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.0f;

    /** Health 0-100 — below 30 triggers limping */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.0f;

    /** Fear 0-100 — affects breathing and idle fidgets */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.0f;

    /** Is the character limping (health < 30)? */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsLimping = false;

    /** Is the character exhausted (stamina < 15)? */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    // ── IK Properties ─────────────────────────────────────────────────────────

    /** Foot IK data computed each frame */
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_FootIKData FootIKData;

    /** Lean data for directional movement */
    UPROPERTY(BlueprintReadOnly, Category = "Lean")
    FAnim_LeanData LeanData;

    /** Look-at pitch for spine aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimPitch = 0.0f;

    /** Look-at yaw for spine aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimYaw = 0.0f;

    // ── Blend Weights ─────────────────────────────────────────────────────────

    /** Upper body additive blend weight (0=lower only, 1=full upper) */
    UPROPERTY(BlueprintReadOnly, Category = "Blend")
    float UpperBodyBlendWeight = 1.0f;

    /** Additive breathing animation alpha */
    UPROPERTY(BlueprintReadOnly, Category = "Blend")
    float BreathingAlpha = 1.0f;

    // ── Blueprint Callable Functions ──────────────────────────────────────────

    /** Trigger jump montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJump();

    /** Trigger land montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLand(float FallDuration);

    /** Trigger melee attack montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerMeleeAttack(int32 ComboIndex);

    /** Trigger death montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDeath();

    /** Trigger hit reaction montage */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerHitReaction(FVector HitDirection);

private:
    // ── Internal helpers ──────────────────────────────────────────────────────

    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateMovementDirection();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateSurvivalState();
    void UpdateBreathing(float DeltaSeconds);

    EAnim_MovementState DetermineMovementState() const;

    // Cached character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    // Smoothed lean values
    float SmoothedLeanFB = 0.0f;
    float SmoothedLeanLR = 0.0f;

    // Breathing oscillator
    float BreathingTime = 0.0f;

    // Previous frame velocity for acceleration detection
    FVector PreviousVelocity = FVector::ZeroVector;

    // Foot IK trace channel
    static constexpr float FootIKTraceDistance = 50.0f;
    static constexpr float FootIKInterpSpeed = 15.0f;
};
