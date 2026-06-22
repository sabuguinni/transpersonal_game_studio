// DinoSurvivorAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for the prehistoric survivor character.
// Drives: locomotion state machine, bilateral foot IK, aim offset, lean, survival posture.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Survival Posture Enum — GLOBAL SCOPE (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAnim_SurvivalPosture : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Injured     UMETA(DisplayName = "Injured")
};

// ─────────────────────────────────────────────────────────────────────────────
// UDinoSurvivorAnimInstance
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    // ── UAnimInstance overrides ──────────────────────────────────────────────
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Blueprint-callable setters ───────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetSurvivalStats(float Health, float Stamina, float Fear);

    // ────────────────────────────────────────────────────────────────────────
    // LOCOMOTION PROPERTIES
    // ────────────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bShouldMove;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bJustLanded;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LandVelocityZ;

    // ────────────────────────────────────────────────────────────────────────
    // AIM OFFSET
    // ────────────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

    // ────────────────────────────────────────────────────────────────────────
    // LEAN
    // ────────────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanAngle;

    // ────────────────────────────────────────────────────────────────────────
    // SURVIVAL POSTURE
    // ────────────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    EAnim_SurvivalPosture SurvivalPosture;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float PostureBlend;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float HealthRatio;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float FearLevel;

    // ────────────────────────────────────────────────────────────────────────
    // FOOT IK
    // ────────────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector PelvisOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float IKInterpSpeed;

private:
    // ── Internal update methods ──────────────────────────────────────────────
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateSurvivalPosture(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    FVector TraceFootIK(FName SocketName, float DeltaSeconds);

    // ── Cached references ────────────────────────────────────────────────────
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovement;

    // ── Frame state ──────────────────────────────────────────────────────────
    float PreviousVelocityZ;
    bool  bWasInAir;
};
