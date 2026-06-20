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
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    InAir       UMETA(DisplayName = "InAir"),
    Land        UMETA(DisplayName = "Land")
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield")
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector EffectorLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float Alpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float HitDistance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bFootOnGround = false;
};

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
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bJustLanded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ── Aim Offset ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw = 0.0f;

    // ── Foot IK ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKTraceDistance = 55.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKInterpSpeed = 15.0f;

    // ── Survival State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearNormalized = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsFleeing = false;

    // ── Combat ───────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_CombatStance CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsBlocking = false;

    // ── Blueprint Events ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatStance(EAnim_CombatStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttack();

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLocomotionBlendWeight() const;

private:
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void SolveFootIK(FAnim_FootIKData& FootData, FName SocketName, float DeltaSeconds);
    EAnim_LocomotionState DetermineLocomotionState() const;

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    float LandedTimer = 0.0f;
    float AttackTimer = 0.0f;
    float PreviousSpeed = 0.0f;
};
