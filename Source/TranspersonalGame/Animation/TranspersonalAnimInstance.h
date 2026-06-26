// TranspersonalAnimInstance.h
// Animation Agent #10 — Prehistoric Survival Game
// UAnimInstance subclass driving locomotion, foot IK, and lean for the player character

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Locomotion state enum — must be at global scope (UE5 RULE 1)
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle    UMETA(DisplayName = "Idle"),
    Walk    UMETA(DisplayName = "Walk"),
    Run     UMETA(DisplayName = "Run"),
    Jump    UMETA(DisplayName = "Jump"),
    Crouch  UMETA(DisplayName = "Crouch"),
    Dead    UMETA(DisplayName = "Dead")
};

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // ─── UAnimInstance overrides ─────────────────────────────────────────────
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion properties (read by AnimBP blend nodes) ─────────────────

    /** Ground speed (cm/s) — drives Walk/Run blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Movement direction relative to character facing (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when character is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when running (speed > WalkSpeed threshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsRunning;

    /** True when fully stationary */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsIdle;

    /** Current discrete locomotion state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState CurrentLocomotionState;

    // ─── Upper body / aim ────────────────────────────────────────────────────

    /** Controller pitch delta for upper-body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float PitchOffset;

    /** Lateral lean angle for turns (-15 to 15 degrees) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    float LeanAngle;

    // ─── Foot IK ─────────────────────────────────────────────────────────────

    /** IK blend weight for left foot (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float FootIK_LeftAlpha;

    /** IK blend weight for right foot (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float FootIK_RightAlpha;

    // ─── Speed thresholds (configurable per character) ───────────────────────

    /** Speed below which character is considered walking */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float WalkSpeed;

    /** Speed above which character is considered running */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float RunSpeed;

private:
    // ─── Cached references ───────────────────────────────────────────────────
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // ─── Internal update helpers ─────────────────────────────────────────────
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
};
