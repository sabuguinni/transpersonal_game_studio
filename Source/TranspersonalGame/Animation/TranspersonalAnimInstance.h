#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;

/**
 * Locomotion state enum for the Animation State Machine
 * Controls which animation state the character is in
 */
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
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death")
};

/**
 * Foot IK data for ground adaptation
 */
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;
};

/**
 * Lean data for directional lean during movement
 */
USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanForward = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanRight = 0.0f;
};

/**
 * TranspersonalAnimInstance
 * 
 * Core Animation Instance for the prehistoric survivor character.
 * Implements Motion Matching-style locomotion with foot IK terrain adaptation.
 * 
 * Features:
 * - Idle/Walk/Run/Sprint/Jump/Fall/Land state machine
 * - Foot IK for uneven prehistoric terrain
 * - Directional lean during movement transitions
 * - Aim offset for weapon/tool aiming
 * - Attack montage integration
 * - Survival state driven animation (exhausted, injured, starving)
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
    virtual void NativeBeginPlay() override;

    // ── Locomotion Properties ──────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float ForwardSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float RightSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsLanding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float AirTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ── Aim Properties ─────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    bool bIsAiming = false;

    // ── Foot IK Properties ─────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    bool bEnableFootIK = true;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKTraceDistance = 55.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKInterpSpeed = 15.0f;

    // ── Lean Properties ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    FAnim_LeanData LeanData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Lean")
    float LeanInterpSpeed = 4.0f;

    // ── Survival State Properties ──────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthRatio = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured = false;

    // ── Attack / Combat Properties ─────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    int32 AttackIndex = 0;

    // ── Speed Thresholds (configurable) ───────────────────────────────────

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float RunSpeedThreshold = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float SprintSpeedThreshold = 450.0f;

    // ── Public Functions ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttack(int32 InAttackIndex);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAiming(bool bAiming);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLocomotionBlendWeight() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayLandAnimation() const;

protected:
    // ── Internal Update Methods ────────────────────────────────────────────

    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAimData(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void UpdateLocomotionState();

    // Foot IK trace helper
    bool TraceFootIK(const FName& FootBoneName, FVector& OutLocation, FRotator& OutRotation);

    // Pelvis adjustment for foot IK
    float CalculatePelvisOffset() const;

private:
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Previous velocity for lean calculation
    FVector PreviousVelocity = FVector::ZeroVector;

    // Landing timer
    float LandingTimer = 0.0f;
    static constexpr float LandingDuration = 0.35f;

    // Air time tracking
    bool bWasInAir = false;
    float AirTimeAccumulator = 0.0f;
};
