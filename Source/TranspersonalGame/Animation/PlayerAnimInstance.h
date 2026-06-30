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
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsClimbing;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float FallSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float JumpPeakAlpha;

    // ── State Machine ────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_WeaponState WeaponState;

    // ── IK ──────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    // ── Survival Stats ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    // ── AimOffset ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

protected:
    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    void UpdateLocomotionState();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void UpdateSurvivalAnimParams();

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    float FootIKTraceDistance;
    float FootIKInterpSpeed;
    float LeanInterpSpeed;
};
