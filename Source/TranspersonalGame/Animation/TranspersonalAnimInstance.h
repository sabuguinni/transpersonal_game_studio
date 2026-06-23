#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

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
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Crouched    UMETA(DisplayName = "Crouched"),
    Prone       UMETA(DisplayName = "Prone"),
    Combat      UMETA(DisplayName = "Combat")
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
struct FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimYaw = 0.0f;
};

/**
 * TranspersonalAnimInstance
 * 
 * Core animation instance for the prehistoric human survivor character.
 * Drives locomotion state machine, foot IK, and upper body layering.
 * Designed for realistic movement with terrain adaptation.
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

    // === LOCOMOTION STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_StanceType StanceType;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionData LocomotionData;

    // === FOOT IK ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKInterpSpeed = 15.0f;

    // === SURVIVAL STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float ExhaustionAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float InjuryAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float FearAlpha = 0.0f;

    // === UPPER BODY LAYERING ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody", meta = (AllowPrivateAccess = "true"))
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody", meta = (AllowPrivateAccess = "true"))
    bool bIsCarryingItem = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|UpperBody", meta = (AllowPrivateAccess = "true"))
    float UpperBodyLayerWeight = 0.0f;

    // === BLEND SPACE INPUTS ===
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float BlendSpaceSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float BlendSpaceDirection = 0.0f;

    // === FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionState();

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalAnimState();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInLocomotionState(EAnim_LocomotionState State) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementDirection() const;

private:
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    void UpdateBlendSpaceInputs();
    void SolveFootIK(bool bIsLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha, float DeltaSeconds);
    EAnim_LocomotionState DetermineLocomotionState() const;
    float CalculatePelvisOffset() const;
};
