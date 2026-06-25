#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

/**
 * FAnim_DinoLocomotionData — runtime locomotion data for dinosaur animation
 * Drives blend spaces: speed, direction, stance
 */
USTRUCT(BlueprintType)
struct FAnim_DinoLocomotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float Speed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float Direction = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    bool bIsAggressive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim|Locomotion")
    float ThreatLevel = 0.f;  // 0=calm, 1=full aggression
};

/**
 * EAnim_DinoStance — high-level stance state for state machine
 */
UENUM(BlueprintType)
enum class EAnim_DinoStance : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Roaring     UMETA(DisplayName = "Roaring"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Dead        UMETA(DisplayName = "Dead"),
};

/**
 * UDinoAnimInstance
 * Base AnimInstance for all dinosaur species.
 * Drives locomotion blend spaces, IK foot placement, and attack montages.
 * Works with any dinosaur skeletal mesh from /Game/Dinosaur_Pack/.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    FAnim_DinoLocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    EAnim_DinoStance CurrentStance = EAnim_DinoStance::Idle;

    // ── IK Foot Placement ─────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK",
              meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK",
              meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK",
              meta = (AllowPrivateAccess = "true"))
    float FootIKAlpha = 1.f;

    // ── Blend Space Inputs ────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace",
              meta = (AllowPrivateAccess = "true"))
    float LocomotionSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|BlendSpace",
              meta = (AllowPrivateAccess = "true"))
    float LocomotionDirection = 0.f;

    // ── Attack / Montage ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bPlayAttackMontage = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bPlayRoarMontage = false;

    // ── Blueprint-callable helpers ────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetLocomotionSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerRoar();

    UFUNCTION(BlueprintPure, Category = "Anim|Locomotion")
    EAnim_DinoStance GetCurrentStance() const { return CurrentStance; }

protected:
    // ── Internal helpers ──────────────────────────────────────────────
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateFootIK();
    void DetermineStance();

    UPROPERTY()
    TObjectPtr<APawn> OwnerPawn;

    float AttackCooldown = 0.f;
    float RoarCooldown   = 0.f;
};
