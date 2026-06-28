#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Combat      UMETA(DisplayName = "Combat"),
    Stealth     UMETA(DisplayName = "Stealth"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

/**
 * UTranspersonalAnimInstance
 * AnimInstance for the player character (TranspersonalCharacter).
 * Drives locomotion blending, IK foot placement, and survival-state poses.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // --- Lifecycle ---
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAngle;

    // --- Survival Stats ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsDead;

    // --- IK Foot Placement ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float PelvisOffset;

    // --- AimOffset ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYaw;

    // --- Blend Weights ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float UpperBodyBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AdditiveWoundBlend;

private:
    // Cached references
    class ATranspersonalCharacter* OwnerCharacter;
    class UCharacterMovementComponent* MovementComponent;

    // Internal helpers
    void UpdateLocomotionState();
    void UpdateSurvivalStats();
    void UpdateIKFootPlacement();
    void UpdateAimOffset();
    void SolveFootIK(FName FootSocketName, FVector& OutIKLocation, float& OutAlpha);
};
