#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// Locomotion state enum for state machine
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

// Foot IK data per foot
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    bool bFootOnGround = false;
};

// Lean data for directional movement
USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanLeftRight = 0.0f;
};

/**
 * UPlayerAnimInstance
 * Animation Instance for the prehistoric human survivor character.
 * Drives locomotion state machine, foot IK, aim offset, and survival state blending.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion Properties ───────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ─── Aim Offset Properties ───────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw = 0.0f;

    // ─── Foot IK Properties ──────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool bEnableFootIK = true;

    // ─── Lean Properties ─────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    FAnim_LeanData LeanData;

    // ─── Survival State Properties ───────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float InjuryBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ExhaustionBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsDead = false;

    // ─── Speed Thresholds ────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float RunSpeedThreshold = 250.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float FootIKInterpSpeed = 15.0f;

    // ─── Blueprint Events ────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnLanded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnJumped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnDeathStarted();

    // ─── Blueprint Callable Utilities ────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetFootIKEnabled(bool bEnabled) { bEnableFootIK = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetInjuryState(float InjuryAlpha, bool bInjured);

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetExhaustionState(float ExhaustionAlpha, bool bExhausted);

private:
    // Internal update helpers
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateSurvivalBlending(float DeltaSeconds);
    EAnim_LocomotionState DetermineLocomotionState() const;

    // Foot IK trace helper
    bool TraceForFootIK(const FName& FootSocketName, FAnim_FootIKData& OutFootData);

    // Cached character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    // Previous frame velocity for lean calculation
    FVector PreviousVelocity = FVector::ZeroVector;

    // Interpolated lean values
    float SmoothedLeanFB = 0.0f;
    float SmoothedLeanLR = 0.0f;
};
