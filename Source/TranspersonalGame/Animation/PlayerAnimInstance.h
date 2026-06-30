#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// ============================================================
// ENUMS — at global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Sneak       UMETA(DisplayName = "Sneak"),
    InAir       UMETA(DisplayName = "In Air"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb")
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "Spear Ready"),
    SpearThrow  UMETA(DisplayName = "Spear Throw"),
    BowReady    UMETA(DisplayName = "Bow Ready"),
    BowDraw     UMETA(DisplayName = "Bow Draw")
};

// ============================================================
// STRUCTS — at global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float PelvisOffset = 0.0f;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --------------------------------------------------------
    // LOCOMOTION PROPERTIES
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float LeanAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsAccelerating;

    // --------------------------------------------------------
    // STATE MACHINE
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_WeaponState WeaponState;

    // --------------------------------------------------------
    // FOOT IK
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData FootIK_Left;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData FootIK_Right;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector PelvisOffset;

    // --------------------------------------------------------
    // SURVIVAL STATS (fed from TranspersonalCharacter)
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    // --------------------------------------------------------
    // BLENDSPACE VALUES
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "BlendSpace")
    float BlendSpace_Speed;

    UPROPERTY(BlueprintReadOnly, Category = "BlendSpace")
    float BlendSpace_Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Aim")
    float AimYaw;

    // --------------------------------------------------------
    // TRANSITION FLAGS
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    float LandingImpact;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bJustLanded;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    bool bJustJumped;

    UPROPERTY(BlueprintReadOnly, Category = "Transitions")
    float TimeInAir;

    // --------------------------------------------------------
    // PUBLIC FUNCTIONS
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetWeaponState(EAnim_WeaponState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSurvivalStats(float Stamina, float Fear, bool bInjured);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLocomotionBlendWeight() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldApplyFootIK() const;

    // --------------------------------------------------------
    // IK CONFIGURATION
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Config")
    float FootIKTraceLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Config")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Config")
    float PelvisIKInterpSpeed;

private:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* OwnerMovement;

    // Internal state tracking
    float PreviousSpeed;
    EAnim_LocomotionState PreviousLocomotionState;
    bool bWasInAir;
    float TimeInCurrentState;

    // Update functions
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAimData(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalAnimState(float DeltaSeconds);
    void DetermineLocomotionState();
    void UpdateBlendSpaceValues(float DeltaSeconds);
    void UpdateTransitionFlags(float DeltaSeconds);
};
