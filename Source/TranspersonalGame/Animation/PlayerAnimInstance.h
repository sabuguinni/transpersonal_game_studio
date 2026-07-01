// PlayerAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for player character: locomotion state machine, foot IK, lean, survival stats

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    // World-space foot target location after IK trace
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector FootLocation = FVector::ZeroVector;

    // Foot rotation to match surface normal
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator FootRotation = FRotator::ZeroRotator;

    // IK blend weight (0 = no IK, 1 = full IK)
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float IKAlpha = 0.0f;

    // Whether this foot has a valid ground contact
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    bool bIsGrounded = false;
};

USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    // Forward/backward lean angle in degrees (positive = forward)
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    float LeanForwardBack = 0.0f;

    // Left/right lean angle in degrees (positive = right)
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    float LeanLeftRight = 0.0f;

    // Overall lean blend weight
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    float LeanAlpha = 0.0f;
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

    // UAnimInstance overrides
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --------------------------------------------------------
    // LOCOMOTION STATE
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

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
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsDead;

    // --------------------------------------------------------
    // LEAN DATA
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    FAnim_LeanData LeanData;

    // --------------------------------------------------------
    // FOOT IK DATA
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FAnim_FootIKData FootIKLeft;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FAnim_FootIKData FootIKRight;

    // Hip vertical offset to accommodate uneven terrain
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float HipOffset;

    // --------------------------------------------------------
    // AIRBORNE DATA
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    float JumpVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    float FallTime;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    bool bJustLanded;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Airborne")
    float LandImpactAlpha;

    // --------------------------------------------------------
    // SURVIVAL STATS (fed from TranspersonalCharacter)
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured;

    // --------------------------------------------------------
    // FOOT IK SETTINGS
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK|Settings")
    float FootIKTraceLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK|Settings")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK|Settings")
    float HipOffsetInterpSpeed;

    // --------------------------------------------------------
    // BLUEPRINT-CALLABLE FUNCTIONS
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Anim|Survival")
    void SetSurvivalStats(float Health, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Anim|Death")
    void TriggerDeathAnimation();

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    EAnim_LocomotionState GetLocomotionState() const;

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    float GetSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    float GetDirection() const;

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    bool GetIsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Anim|FootIK")
    FAnim_FootIKData GetLeftFootIKData() const;

    UFUNCTION(BlueprintPure, Category = "Anim|FootIK")
    FAnim_FootIKData GetRightFootIKData() const;

    UFUNCTION(BlueprintPure, Category = "Anim|Lean")
    FAnim_LeanData GetLeanData() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwningCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal update methods
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAirborneData(float DeltaSeconds);
    void UpdateLeanData(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLocomotionState();
    void ProcessFootIKTrace(UWorld* World, const FVector& FootLocation, FAnim_FootIKData& FootData, float DeltaSeconds);

    // Internal state
    float TargetHipOffset;
};
