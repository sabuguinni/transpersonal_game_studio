// DinoSurvivorAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// AnimInstance for the prehistoric survivor character
// Locomotion state machine + foot IK + survival posture blending

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalPosture : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector LeftFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector RightFootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator LeftFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator RightFootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisOffset;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bLeftFootGrounded;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bRightFootGrounded;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float LeftFootAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float RightFootAlpha;

    FAnim_FootIKData()
        : LeftFootLocation(FVector::ZeroVector)
        , RightFootLocation(FVector::ZeroVector)
        , LeftFootRotation(FRotator::ZeroRotator)
        , RightFootRotation(FRotator::ZeroRotator)
        , PelvisOffset(0.0f)
        , bLeftFootGrounded(true)
        , bRightFootGrounded(true)
        , LeftFootAlpha(1.0f)
        , RightFootAlpha(1.0f)
    {}
};

USTRUCT(BlueprintType)
struct FAnim_AimOffsetData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimYaw;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    bool bIsAiming;

    FAnim_AimOffsetData()
        : AimYaw(0.0f)
        , AimPitch(0.0f)
        , AimAlpha(0.0f)
        , bIsAiming(false)
    {}
};

// ============================================================
// MAIN ANIMINSTANCE CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Locomotion State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState CurrentLocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAmount;

    // ---- Survival Posture ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    EAnim_SurvivalPosture CurrentSurvivalPosture;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float InjuryLevel;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisInterpSpeed;

    // ---- Aim Offset ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    FAnim_AimOffsetData AimOffsetData;

    // ---- Speed Thresholds ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float SprintSpeed;

    // ---- Blueprint-callable functions ----
    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetSurvivalStats(float Stamina, float Fear, float Injury);

    UFUNCTION(BlueprintCallable, Category = "Animation|Events")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation|Events")
    void TriggerLandAnimation();

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    EAnim_LocomotionState GetLocomotionState() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    float GetGroundSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    bool GetIsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FAnim_FootIKData GetFootIKData() const;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateSurvivalPosture();
};
