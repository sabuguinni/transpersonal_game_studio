#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

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
    CrouchWalk  UMETA(DisplayName = "CrouchWalk")
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow")
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
    float HipOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsInjured = false;
};

/**
 * Animation Instance for the Prehistoric Survivor player character.
 * Drives locomotion state machine, foot IK, and survival-state-driven additive layers.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ── Combat ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat",
        meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat",
        meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    // ── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK",
        meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootIKTraceDistance = 55.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|IK")
    float FootIKInterpSpeed = 15.0f;

    // ── Survival State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    FAnim_SurvivalState SurvivalState;

    // ── Aim Offset ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
        meta = (AllowPrivateAccess = "true"))
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
        meta = (AllowPrivateAccess = "true"))
    float AimYaw = 0.0f;

    // ── Blend Weights ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
        meta = (AllowPrivateAccess = "true"))
    float ExhaustionBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
        meta = (AllowPrivateAccess = "true"))
    float InjuryBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend",
        meta = (AllowPrivateAccess = "true"))
    float FearBlendWeight = 0.0f;

    // ── Blueprint-callable helpers ────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatStance(EAnim_CombatStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttack();

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeedNormalized() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMovingOnGround() const;

private:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalBlends(float DeltaSeconds);
    void UpdateAimOffset();
    bool TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    float AttackCooldownTimer = 0.0f;
    static constexpr float AttackCooldownDuration = 0.5f;
};
