#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation instance for the prehistoric human survivor character.
 * Drives blend spaces for locomotion (idle/walk/run/crouch/jump)
 * and montage slots for attacks, interactions, and reactions.
 */
UCLASS()
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Ground speed (0=Idle, 150=Walk, 400=Run) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed = 0.f;

    /** Lateral strafe direction (-1 left, 0 straight, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float StrafeDirection = 0.f;

    /** Character is in the air (jump / fall) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir = false;

    /** Character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching = false;

    /** Character is sprinting */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting = false;

    /** Character is climbing a surface */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsClimbing = false;

    // ── Combat ──────────────────────────────────────────────────────────────

    /** Currently in a melee attack montage */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking = false;

    /** Weapon type (0=Unarmed, 1=Spear, 2=Club, 3=Bow) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    int32 WeaponType = 0;

    // ── Survival State ───────────────────────────────────────────────────────

    /** Stamina ratio 0-1 — affects animation blend weight for exhaustion */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaRatio = 1.f;

    /** Health ratio 0-1 — below 0.3 triggers limping blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthRatio = 1.f;

    /** Character is limping (HealthRatio < 0.3) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsLimping = false;

    // ── IK ──────────────────────────────────────────────────────────────────

    /** Left foot IK target world position */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKTarget = FVector::ZeroVector;

    /** Right foot IK target world position */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKTarget = FVector::ZeroVector;

    /** Pelvis offset for foot IK correction */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float PelvisOffset = 0.f;

    // ── Aim Offset ──────────────────────────────────────────────────────────

    /** Aim pitch (-90 to 90) for upper body aim offset */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimPitch = 0.f;

    /** Aim yaw (-90 to 90) for upper body aim offset */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYaw = 0.f;

protected:
    /** Cached owning pawn */
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    /** Update foot IK targets via line traces */
    void UpdateFootIK();

    /** Smooth interpolation speed for IK targets */
    float IKInterpSpeed = 15.f;
};
