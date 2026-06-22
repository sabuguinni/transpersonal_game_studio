// DinoSurvivorAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for the prehistoric survivor character

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// Survival posture enum — drives additive pose blending
UENUM(BlueprintType)
enum class EAnim_SurvivalPosture : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Locomotion ----
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
    bool bShouldMove;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsJumping;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsLanding;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float JumpStartSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float TimeSinceLastLand;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector PelvisOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|FootIK")
    float FootIKInterpSpeed;

    // ---- Aim Offset ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYawDelta;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    bool bIsAiming;

    // ---- Survival Posture ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Posture")
    EAnim_SurvivalPosture CurrentPosture;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Posture")
    float PostureBlendAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Posture")
    float SurvivalHealthRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Posture")
    float SurvivalStaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Posture")
    bool bIsFleeing;

    // ---- Lean ----
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Lean")
    float LeanAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Lean")
    float LeanInterpSpeed;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Anim|Survival")
    void SetSurvivalStats(float HealthRatio, float StaminaRatio, bool bFleeing);

    UFUNCTION(BlueprintCallable, Category = "Anim|Aim")
    void SetAimingState(bool bAiming);

protected:
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateSurvivalPosture(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);

    FVector TraceFootIK(UWorld* World, const FName& FootBoneName, FVector& OutFootOffset, float DeltaSeconds);

private:
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* OwnerMovement;

    float PreviousSpeed;
    FVector PreviousVelocityDirection;
    float PreviousAimYaw;
};
