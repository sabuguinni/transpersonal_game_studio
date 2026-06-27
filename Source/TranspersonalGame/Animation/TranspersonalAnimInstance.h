#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation states for the prehistoric survivor character
 */
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim"),
};

/**
 * Upper body overlay states (layered on top of locomotion)
 */
UENUM(BlueprintType)
enum class EAnim_UpperBodyState : uint8
{
    None            UMETA(DisplayName = "None"),
    Carry           UMETA(DisplayName = "Carry Object"),
    Aim             UMETA(DisplayName = "Aim Spear"),
    Throw           UMETA(DisplayName = "Throw"),
    Craft           UMETA(DisplayName = "Crafting"),
    Eat             UMETA(DisplayName = "Eating"),
    Injured         UMETA(DisplayName = "Injured"),
};

/**
 * Locomotion data passed to the animation graph each frame
 */
USTRUCT(BlueprintType)
struct FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float AimYaw = 0.0f;
};

/**
 * IK foot placement data
 */
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootAlpha = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootAlpha = 1.0f;
};

/**
 * UTranspersonalAnimInstance
 * 
 * Core AnimInstance for the prehistoric survivor character.
 * Drives locomotion blend spaces, upper body overlays, and foot IK.
 * Designed for Motion Matching compatibility.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ─── Locomotion State ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    // ─── Upper Body ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody")
    EAnim_UpperBodyState UpperBodyState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody")
    float AimYaw = 0.0f;

    // ─── Foot IK ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootIKTraceDistance = 55.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootIKInterpSpeed = 15.0f;

    // ─── Survival State ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted = false;

    // ─── Blend Weights ───────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float WalkRunBlend = 0.0f;  // 0=walk, 1=run

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float InjuredBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float LandRecoveryAlpha = 0.0f;

    // ─── Blueprint-callable helpers ──────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetUpperBodyState(EAnim_UpperBodyState NewState);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayLandAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLocomotionPlayRate() const;

protected:
    // Internal update helpers
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalState();
    void UpdateBlendWeights(float DeltaSeconds);

    EAnim_LocomotionState DetermineLocomotionState() const;

    // Foot IK trace helper
    bool TraceFootIK(const FName& FootBoneName, FVector& OutLocation, FRotator& OutRotation);

private:
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    float PreviousSpeed = 0.0f;
    float LandRecoveryTimer = 0.0f;
    bool bWasInAir = false;
};
