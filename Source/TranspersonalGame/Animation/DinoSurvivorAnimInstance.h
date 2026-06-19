#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal Survivor (player character).
 * Drives idle/walk/run/jump/crouch states via blend spaces.
 * Foot IK is applied per-frame to adapt to uneven prehistoric terrain.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ────────────────────────────────────────────────────────────

    /** Ground speed (cm/s) — drives the walk/run blend space axis */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    /** Lateral direction offset (-180..180) — used for strafe blending */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Direction;

    /** True while the character is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** True while the character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    /** True when the character is sprinting (speed > SprintThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    /** True when the character is completely still */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsIdle;

    // ── Survival State ────────────────────────────────────────────────────────

    /** Fear level 0-1 — affects animation urgency/trembling */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    /** Stamina 0-1 — affects run posture when low */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    float StaminaRatio;

    /** Health 0-1 — triggers wounded limp below 0.3 */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    float HealthRatio;

    /** True when health < 0.3 — activates wounded locomotion layer */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    bool bIsWounded;

    // ── Foot IK ───────────────────────────────────────────────────────────────

    /** World-space target for left foot IK */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKTarget;

    /** World-space target for right foot IK */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKTarget;

    /** Pelvis vertical offset to keep feet on ground */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    /** Alpha for foot IK blending (0=off, 1=full) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    float FootIKAlpha;

    // ── Thresholds (configurable) ─────────────────────────────────────────────

    /** Speed above which the character is considered sprinting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float SprintThreshold;

    /** Speed below which the character is considered idle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float IdleThreshold;

    /** Trace distance for foot IK ground detection */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float FootIKTraceDistance;

protected:
    /** Cached owner pawn */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Update locomotion variables from movement component */
    void UpdateLocomotionVariables();

    /** Update survival state from character stats */
    void UpdateSurvivalVariables();

    /** Perform foot IK traces and compute pelvis offset */
    void UpdateFootIK();

    /** Trace a single foot to the ground, returns hit world location */
    FVector TraceFootToGround(const FName& FootSocketName, float& OutHitZ) const;
};
