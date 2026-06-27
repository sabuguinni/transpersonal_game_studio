#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Animation states for the survivor character
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
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim"),
    Dead        UMETA(DisplayName = "Dead")
};

// Combat stance states
UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield")
};

// Foot IK data per foot
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float EffectorOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    bool bFootOnGround = true;
};

// Locomotion blend space parameters
USTRUCT(BlueprintType)
struct FAnim_LocomotionParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching = false;
};

/**
 * TranspersonalAnimInstance
 * 
 * Main AnimInstance for the prehistoric survivor character.
 * Drives locomotion state machine, foot IK, and combat stances.
 * Designed for Motion Matching integration with UE5.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // --- LOCOMOTION STATE ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance CombatStance;

    // --- LOCOMOTION PARAMS ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionParams LocomotionParams;

    // --- SPEED & DIRECTION ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity;

    // --- AIRBORNE ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air", meta = (AllowPrivateAccess = "true"))
    float TimeInAir;

    // --- FOOT IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    // --- AIM OFFSET ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float AimYaw;

    // --- SURVIVAL STATES ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float HealthRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsWounded;

    // --- LAND TIMER ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    float LandTimer;

    // --- THRESHOLDS ---
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float SprintSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float LandRecoveryTime;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float FootIKTraceDistance;

    // --- BLUEPRINT EVENTS ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnLanded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnJumped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnExhausted();

    // --- UTILITY ---
    UFUNCTION(BlueprintCallable, Category = "Animation|Utility")
    EAnim_LocomotionState GetCurrentLocomotionState() const { return LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation|Utility")
    bool IsInCombatStance() const { return CombatStance != EAnim_CombatStance::Unarmed; }

    UFUNCTION(BlueprintCallable, Category = "Animation|Utility")
    void SetCombatStance(EAnim_CombatStance NewStance);

private:
    // Internal update methods
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void PerformFootTrace(const FName& FootBoneName, FAnim_FootIKData& OutFootData);

    // Cached character reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Previous state for transition detection
    EAnim_LocomotionState PreviousLocomotionState;
    bool bWasInAir;
    float FootIKAlpha;
};
