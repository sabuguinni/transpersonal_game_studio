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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_StanceType StanceType;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    // ── Foot IK ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootEffectorLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootEffectorLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool bEnableFootIK;

    // ── Survival Stats ───────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    // ── Aim Offsets ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimYaw;

    // ── Blend Space Inputs ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float BlendSpaceSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float BlendSpaceDirection;

protected:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalStats();
    void UpdateAimOffsets();

    void PerformFootTrace(
        const FName& FootSocketName,
        FVector& OutEffectorLocation,
        FRotator& OutFootRotation);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    static constexpr float WalkSpeedThreshold    = 10.0f;
    static constexpr float SprintSpeedThreshold  = 350.0f;
    static constexpr float FootTraceLength       = 50.0f;
    static constexpr float FootIKInterpSpeed     = 15.0f;
};
