#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnimInstance_Survivor.generated.h"

/**
 * Animation Instance for the prehistoric human survivor character.
 * Drives the locomotion blend space, IK foot placement, and montage system.
 * Designed for Motion Matching-style transitions between survival states.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimInstance_Survivor : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnimInstance_Survivor();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ────────────────────────────────────────────────────────────

    /** Current movement speed (cm/s). Drives the locomotion blend space axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    /** Lateral movement direction (-1 = left, 0 = forward, 1 = right). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    /** True when the character is in the air (jumped or fell). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    /** True when the character is sprinting (speed > 400 cm/s). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    /** True when the character is moving (speed > 10 cm/s). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    // ─── IK Foot Placement ─────────────────────────────────────────────────────

    /** World-space location for the left foot IK target. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** World-space location for the right foot IK target. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Pelvis vertical offset to keep feet grounded on uneven terrain. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    /** Alpha blend for IK foot placement (0 = off, 1 = full IK). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float FootIKAlpha;

    // ─── Survival States ───────────────────────────────────────────────────────

    /** Current stamina (0-100). Affects animation speed and blend weights. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Stamina;

    /** Current health (0-100). Low health triggers injured animation layer. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Health;

    /** True when health < 30 — activates injured locomotion overlay. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    /** True when carrying a heavy object — affects posture and speed. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsCarryingHeavy;

    // ─── Combat ────────────────────────────────────────────────────────────────

    /** True when in combat stance (weapon drawn or threat detected). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsInCombatStance;

    /** True when performing an attack action. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking;

    /** Aim pitch offset for upper body aiming (-90 to 90 degrees). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimPitch;

    /** Aim yaw offset for upper body rotation. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimYaw;

    // ─── Montage Triggers ──────────────────────────────────────────────────────

    /** Play a specific animation montage by name. Called from character BP. */
    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void PlaySurvivorMontage(const FName& MontageName);

    /** Stop the currently playing montage with a blend-out time. */
    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    void StopCurrentMontage(float BlendOutTime = 0.25f);

    // ─── IK Calculation ────────────────────────────────────────────────────────

    /** Perform a line trace to find the ground position for foot IK. */
    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(float DeltaSeconds);

private:
    /** Cached reference to the owning character. */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached reference to the character movement component. */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Smooth interpolation speed for pelvis offset correction. */
    float PelvisInterpSpeed;

    /** Trace distance for foot IK ground detection. */
    float FootTraceDistance;

    /** Internal: calculate pelvis offset from foot IK targets. */
    float CalculatePelvisOffset() const;
};
