#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle          UMETA(DisplayName = "Idle"),
    Walk          UMETA(DisplayName = "Walk"),
    Run           UMETA(DisplayName = "Run"),
    Sprint        UMETA(DisplayName = "Sprint"),
    Jump          UMETA(DisplayName = "Jump"),
    Fall          UMETA(DisplayName = "Fall"),
    Land          UMETA(DisplayName = "Land"),
    Crouch        UMETA(DisplayName = "Crouch"),
    CrouchWalk    UMETA(DisplayName = "CrouchWalk"),
    Dead          UMETA(DisplayName = "Dead")
};

// ============================================================
// Structs — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bLeftFootGrounded = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bRightFootGrounded = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanLeftRight = 0.0f;
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
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimYaw = 0.0f;
};

// ============================================================
// UPlayerAnimInstance — Main Animation Instance
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ---- Locomotion State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionData LocomotionData;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    // ---- Lean ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean", meta = (AllowPrivateAccess = "true"))
    FAnim_LeanData LeanData;

    // ---- Raw values for BlendSpace driving ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    // ---- Survival stat influence on animation ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    // ---- Foot IK configuration ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|FootIK|Config")
    float FootIKTraceDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|FootIK|Config")
    float FootIKInterpSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|FootIK|Config")
    float PelvisAdjustmentSpeed;

    // ---- Speed thresholds ----
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float WalkSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float RunSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float SprintSpeed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float MinMovingSpeed;

    // ---- Blueprint callable utilities ----
    UFUNCTION(BlueprintCallable, Category = "Animation|State")
    EAnim_LocomotionState GetLocomotionState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    FAnim_FootIKData GetFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "Animation|Lean")
    FAnim_LeanData GetLeanData() const;

    UFUNCTION(BlueprintPure, Category = "Animation|State")
    bool IsInLocomotionState(EAnim_LocomotionState State) const;

private:
    // Cached owner character
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal update helpers
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateSurvivalInfluence();

    // Foot IK helpers
    bool TraceFootPosition(const FName& FootSocketName, FVector& OutLocation, FRotator& OutRotation);
    float CalculatePelvisOffset() const;

    // Lean smoothing
    float PreviousVelocityX;
    float PreviousVelocityY;
    float LeanSmoothing;
};
