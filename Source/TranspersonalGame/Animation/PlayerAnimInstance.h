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
    Climb       UMETA(DisplayName = "Climb"),
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield"),
    Torch       UMETA(DisplayName = "Torch"),
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float PelvisOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Health = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Stamina = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Hunger = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsWounded = false;
};

/**
 * UPlayerAnimInstance
 * Main AnimInstance for the prehistoric human survivor character.
 * Drives locomotion blend space, foot IK, survival state overlays,
 * and combat stance transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float SmoothedSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSneaking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ─── Combat ───────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_CombatStance CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsBlocking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AttackPlayRate = 1.0f;

    // ─── Survival overlays ────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    FAnim_SurvivalState SurvivalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ExhaustionAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float WoundedAlpha = 0.0f;

    // ─── Foot IK ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootTraceDistance = 50.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    bool bEnableFootIK = true;

    // ─── Aim offset ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw = 0.0f;

    // ─── Thresholds (configurable) ────────────────────────────────
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float SprintSpeedThreshold = 550.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float SpeedSmoothingRate = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float LeanSmoothingRate = 5.0f;

    // ─── Blueprint events ─────────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnLanded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnJumped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnAttackStarted();

    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void SetCombatStance(EAnim_CombatStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void TriggerAttack(float PlayRate = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    float GetMovementDirection() const;

private:
    void UpdateLocomotionState();
    void UpdateSurvivalOverlays(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void SmoothLocomotionValues(float DeltaSeconds);

    bool PerformFootTrace(const FName& SocketName, FVector& OutHitLocation, FRotator& OutHitNormal) const;

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    float PreviousSpeed = 0.0f;
    float LandedTimer = 0.0f;
    bool bWasInAir = false;
};
