#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation state enum for the TranspersonalCharacter state machine.
 * Covers all locomotion states needed for a prehistoric survival character.
 */
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
    Death       UMETA(DisplayName = "Death")
};

/**
 * Foot IK data for two-bone IK foot placement on uneven terrain.
 */
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

/**
 * Lean data for additive lean animation during turns and strafing.
 */
USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanForwardBack = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanLeftRight = 0.0f;
};

/**
 * UTranspersonalAnimInstance — the core AnimInstance for the player character.
 *
 * Drives the locomotion state machine with:
 *   - Speed-based state transitions (idle/walk/run/sprint)
 *   - In-air detection (jump/fall/land)
 *   - Foot IK for terrain adaptation
 *   - Additive lean during movement
 *   - Montage slot support for attacks and interactions
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
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion State ───────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    // ─── Foot IK ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
        meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|FootIK")
    float FootIKInterpSpeed;

    // ─── Lean ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean",
        meta = (AllowPrivateAccess = "true"))
    FAnim_LeanData LeanData;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Lean")
    float LeanInterpSpeed;

    // ─── Speed Thresholds ───────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Thresholds")
    float SprintSpeedThreshold;

    // ─── Survival State (drives animation overlays) ─────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

private:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLean(float DeltaSeconds);
    void UpdateSurvivalOverlays();

    FVector PreviousVelocity;
    float LandingTimer;

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;
};
