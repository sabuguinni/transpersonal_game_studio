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
 * Animation instance for the prehistoric survivor character.
 * Drives locomotion blending, foot IK, and survival state animations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ───────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAngle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    // ─── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootEffectorLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootEffectorLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator LeftFootRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator RightFootRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    float IKTraceDistance;

    // ─── Survival State ───────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float HealthNormalized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsWounded;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsDead;

    // ─── Aim / Upper Body ─────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimPitch;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYaw;

    // ─── Internal helpers ─────────────────────────────────────────────────────

protected:
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    EAnim_LocomotionState DetermineLocomotionState() const;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    FVector PreviousVelocity;
    float LeanAlpha;
};
