#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Animation states for the prehistoric survivor character
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Attack      UMETA(DisplayName = "Attack"),
    Dodge       UMETA(DisplayName = "Dodge"),
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

// Survival state data fed from TranspersonalCharacter
USTRUCT(BlueprintType)
struct FAnim_SurvivalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsWounded = false;
};

/**
 * Animation Instance for the prehistoric human survivor character.
 * Drives locomotion state machine, foot IK, and survival-reactive animations.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ─── Locomotion ───────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAngle = 0.0f;

    // ─── Foot IK ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;

    // ─── Survival Reactivity ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    FAnim_SurvivalState SurvivalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ExhaustionBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float WoundedLimping = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearTremor = 0.0f;

    // ─── Attack / Combat ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float UpperBodyLayerWeight = 0.0f;

    // ─── Utility ──────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpMontage();

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeedNormalized() const;

private:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalReactivity();
    void SolveFootIK(const FName& FootBoneName, FAnim_FootIKData& OutIKData, float DeltaSeconds);

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // IK trace settings
    float FootTraceLength = 50.0f;
    float FootIKInterpSpeed = 15.0f;
    float PelvisInterpSpeed = 10.0f;
};
