// PlayerAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260701_004
// Complete animation instance for prehistoric survivor character

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerAnimInstance.generated.h"

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
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Dead        UMETA(DisplayName = "Dead")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector IKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FRotator IKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool bGroundContact = false;
};

USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanLeftRight = 0.0f;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // ---- Locomotion State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

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

    // ---- Speed Thresholds ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float SprintSpeedThreshold;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    bool bFootIKEnabled;

    // ---- Lean ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    FAnim_LeanData LeanData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Lean")
    float LeanInterpSpeed;

    // ---- Aim Offset ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

    // ---- Jump / Land ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
    float JumpVelocityZ;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
    bool bJustLanded;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Jump")
    float LandImpactAlpha;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnLanded(const FHitResult& Hit);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FString GetLocomotionStateName() const;

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSingleFootIK(FAnim_FootIKData& FootData, FName SocketName, float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateJumpLand(float DeltaSeconds);

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* OwnerMovement;
};
