#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivalAnimInstance.generated.h"

/**
 * Animation Instance for the prehistoric survivor player character.
 * Drives idle/walk/run/sprint/jump/crouch state machine via blend spaces.
 * Agent #10 — Animation Agent
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Current movement speed (cm/s) — drives blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Lateral strafe direction (-1 left, 0 forward, +1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when character is sprinting (speed > SprintThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** True when character is moving (speed > MovingThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    // ── Survival Stats ───────────────────────────────────────────────────────

    /** Stamina 0-100 — affects animation weight/fatigue blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float Stamina;

    /** Fear level 0-100 — triggers fear locomotion blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    /** True when near a predator (triggers alert idle) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsThreatened;

    // ── Combat ───────────────────────────────────────────────────────────────

    /** True when character is in combat stance */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsInCombat;

    /** True when attacking — triggers attack montage */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** True when carrying a tool/weapon */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsArmed;

    // ── IK ───────────────────────────────────────────────────────────────────

    /** Left foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    /** Pelvis offset for foot IK adjustment */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float PelvisOffset;

    // ── Thresholds ───────────────────────────────────────────────────────────

    /** Speed above which character is considered moving */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float MovingThreshold;

    /** Speed above which character is considered sprinting */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float SprintThreshold;

    /** Fear level above which bIsThreatened is true */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float ThreatFearThreshold;

protected:
    /** Cached reference to owning character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Update foot IK traces */
    void UpdateFootIK(float DeltaSeconds);

    /** Perform a line trace for foot placement */
    bool TraceFootIK(const FName& FootSocketName, FVector& OutLocation);
};
