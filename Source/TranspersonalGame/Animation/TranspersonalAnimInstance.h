#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal prehistoric survival game character.
 * Drives locomotion blend space, jump states, and survival-state driven poses.
 * Agent #10 — Animation Agent
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

    /** Ground speed (0 = idle, >0 = moving). Drives BlendSpace X axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed;

    /** Lateral strafe speed. Drives BlendSpace Y axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float StrafeSpeed;

    /** True when the character is accelerating (not decelerating to a stop). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsAccelerating;

    /** True when the character is in the air (jumping or falling). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    /** True when the character is sprinting (speed > SprintThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    /** Direction of movement relative to actor forward (-180 to 180). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection;

    // ── Survival States ─────────────────────────────────────────────────────

    /** Stamina 0-1. Below 0.2 triggers exhaustion pose blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;

    /** Health 0-1. Below 0.3 triggers injured locomotion blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthNormalized;

    /** Fear level 0-1. Above 0.7 triggers fear locomotion blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearNormalized;

    /** True when character is carrying a heavy object (changes posture). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsCarryingHeavyObject;

    // ── IK ──────────────────────────────────────────────────────────────────

    /** Foot IK enabled when on uneven terrain. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bEnableFootIK;

    /** Left foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Pelvis adjustment offset for foot IK. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    // ── Thresholds ───────────────────────────────────────────────────────────

    /** Speed above which character is considered sprinting. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float SprintThreshold;

    /** Speed above which character is considered walking (not idle). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float WalkThreshold;

protected:
    /** Cached reference to owning character. */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component. */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    /** Update locomotion variables from movement component. */
    void UpdateLocomotionData(float DeltaSeconds);

    /** Update survival state variables from character stats. */
    void UpdateSurvivalData();

    /** Perform foot IK raycasts and update IK targets. */
    void UpdateFootIK();

    /** Calculate movement direction angle relative to actor forward. */
    float CalculateMovementDirection() const;

    /** Smoothly interpolate pelvis offset for foot IK. */
    float CurrentPelvisOffset;

    /** Smooth speed for pelvis IK interpolation. */
    static constexpr float PelvisInterpSpeed = 15.0f;
};
