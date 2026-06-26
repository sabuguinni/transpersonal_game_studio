// TranspersonalAnimInstance.h
// Animation Agent #10 — Cycle PROD_CYCLE_AUTO_20260626_004
// AnimInstance for prehistoric survivor character — locomotion blend + foot IK + survival state

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Locomotion state enum — used by AnimBP state machine
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    InAir       UMETA(DisplayName = "InAir"),
};

// Foot IK data per foot
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootOffset = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // --- Locomotion Properties ---

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsIdle;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_LocomotionState LocomotionState;

    // --- Foot IK Properties ---

    UPROPERTY(BlueprintReadOnly, Category = "FootIK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "FootIK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "FootIK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "FootIK")
    FVector RightFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FootIK")
    float IKInterpSpeed;

    // --- Survival State Properties ---

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsWounded;

    // --- Blueprint Callable Utilities ---

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetLocomotionPlayRate() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const;

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    void UpdateSurvivalState();
    void UpdateFootIK(float DeltaSeconds);

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* OwnerMovement;
};
