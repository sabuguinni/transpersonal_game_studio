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
    InAir       UMETA(DisplayName = "InAir")
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────
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
    EAnim_LocomotionState LocomotionState;

    // ── Combat ───────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    EAnim_CombatStance CombatStance;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsBlocking;

    // ── Survival ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float HealthRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured;

    // ── IK ───────────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float IKAlpha;

    // ── Lean / AimOffset ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float AimYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset")
    float LeanAngle;

    // ── Thresholds (designer-tunable) ────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float SprintSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Config")
    float IKTraceDistance;

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void SetCombatStance(EAnim_CombatStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerAttack();

private:
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateCombatState(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    float AttackCooldown;
    float PreviousSpeed;
};
