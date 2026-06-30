#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Sneak       UMETA(DisplayName = "Sneak"),
    InAir       UMETA(DisplayName = "InAir"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb")
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "SpearReady"),
    SpearThrow  UMETA(DisplayName = "SpearThrow"),
    BowReady    UMETA(DisplayName = "BowReady"),
    BowDraw     UMETA(DisplayName = "BowDraw")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity;

    // ── Weapon / Combat ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_WeaponState WeaponState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimYaw;

    // ── Survival Stats ──────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;   // 0.0 – 1.0

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthNormalized;    // 0.0 – 1.0

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    // ── IK ──────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float FootIKAlpha;

    // ── Thresholds ──────────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float SprintSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float IKTraceDistance;

private:
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    void UpdateLocomotionState();
    void UpdateWeaponState();
    void UpdateSurvivalStats();
    void UpdateFootIK();
    FVector TraceFootIK(const FName& FootSocketName) const;
};
