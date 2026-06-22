#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// Locomotion state enum — drives the state machine
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

// Survival state — modifies upper body posture
UENUM(BlueprintType)
enum class EAnim_SurvivalPosture : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Tired       UMETA(DisplayName = "Tired"),
    Injured     UMETA(DisplayName = "Injured"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Exhausted   UMETA(DisplayName = "Exhausted")
};

// Foot IK data for bilateral ground adaptation
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
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float RightFootAlpha = 0.0f;
};

// Aim offset data for upper body aiming
USTRUCT(BlueprintType)
struct FAnim_AimOffsetData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimYaw = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimAlpha = 0.0f;
};

/**
 * DinoSurvivorAnimInstance
 * 
 * Core AnimInstance for the prehistoric human survivor character.
 * Drives locomotion state machine, foot IK ground adaptation,
 * aim offset, and survival posture blending.
 * 
 * Architecture:
 * - Locomotion state machine: Idle/Walk/Run/Sprint/Crouch/Jump/Fall/Land
 * - Foot IK: bilateral line traces + pelvis offset for terrain adaptation
 * - Aim offset: pitch/yaw interpolation for weapon/tool aiming
 * - Survival blending: stamina/health/fear → upper body weight modulation
 * - Lean: velocity delta → lateral lean angle
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float MovementDirection = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity = 0.0f;

    // ─── Survival Posture ─────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalPosture SurvivalPosture = EAnim_SurvivalPosture::Healthy;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float SurvivalBlendWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float FearIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float HealthNormalized = 1.0f;

    // ─── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKTraceDistance = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    bool bEnableFootIK = true;

    // ─── Aim Offset ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    FAnim_AimOffsetData AimOffsetData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|AimOffset")
    float AimInterpSpeed = 10.0f;

    // ─── Speed thresholds ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float RunSpeedThreshold = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Thresholds")
    float SprintSpeedThreshold = 450.0f;

    // ─── Blueprint callable helpers ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetGroundSpeed() const { return GroundSpeed; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool GetIsMoving() const { return bIsMoving; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_FootIKData GetFootIKData() const { return FootIKData; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_AimOffsetData GetAimOffsetData() const { return AimOffsetData; }

private:
    // Internal update helpers
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateSurvivalPosture(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);

    // Foot IK line trace helper
    bool TraceFootIK(const FName& SocketName, FVector& OutLocation, FRotator& OutRotation);

    // Cached character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    // Previous velocity for lean calculation
    FVector PreviousVelocity = FVector::ZeroVector;

    // Interpolated pelvis offset
    float CurrentPelvisOffset = 0.0f;

    // Interpolated foot locations
    FVector CurrentLeftFootLoc = FVector::ZeroVector;
    FVector CurrentRightFootLoc = FVector::ZeroVector;
};
