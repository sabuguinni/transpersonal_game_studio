#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;

/** Locomotion state enum for the animation state machine */
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
    Death       UMETA(DisplayName = "Death")
};

/** Foot IK data for a single foot */
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    float EffectorOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Foot IK")
    bool bFootOnGround = true;
};

/**
 * UTranspersonalAnimInstance
 * Animation instance for the prehistoric survivor character.
 * Drives locomotion state machine, foot IK, and montage slots.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    /** Called every frame to update animation variables */
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Called on initialization */
    virtual void NativeInitializeAnimation() override;

    // ─── Locomotion Variables ───────────────────────────────────────────

    /** Current movement speed (0 = idle, 600 = sprint) */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed = 0.0f;

    /** Lateral direction for strafing (-1 left, 0 center, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction = 0.0f;

    /** Is the character in the air? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir = false;

    /** Is the character crouching? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching = false;

    /** Is the character sprinting? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSprinting = false;

    /** Is the character dead? */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsDead = false;

    /** Current locomotion state for state machine transitions */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    /** Vertical velocity for jump/fall blend */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float VerticalVelocity = 0.0f;

    /** Acceleration magnitude for lean/tilt effects */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float AccelerationMagnitude = 0.0f;

    // ─── Foot IK Variables ───────────────────────────────────────────────

    /** Left foot IK data */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    /** Right foot IK data */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    /** Pelvis offset to prevent body from floating above uneven terrain */
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset = 0.0f;

    // ─── Combat Variables ────────────────────────────────────────────────

    /** Is the character in combat stance? */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

    /** Upper body layer weight for weapon overlay */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float UpperBodyLayerWeight = 0.0f;

    // ─── Survival Variables ──────────────────────────────────────────────

    /** Health ratio 0-1 — affects movement animations (limping at low health) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthRatio = 1.0f;

    /** Stamina ratio 0-1 — affects run speed and breathing */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaRatio = 1.0f;

    /** Fear level 0-1 — affects posture and movement jitter */
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    // ─── Blueprint Callable Functions ────────────────────────────────────

    /** Set the character as dead — triggers death animation */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetDead(bool bDead);

    /** Set combat stance */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatStance(bool bCombat);

    /** Update survival stats from character */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats(float Health, float Stamina, float Fear);

protected:
    /** Perform foot IK trace for a single foot */
    void UpdateFootIK(const FName& FootBoneName, FAnim_FootIKData& OutFootData);

    /** Update locomotion state from movement component */
    void UpdateLocomotionState();

    /** Cached owning character */
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    /** Cached movement component */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    /** Foot IK trace distance (half-height of character + extra) */
    UPROPERTY(EditDefaultsOnly, Category = "Foot IK")
    float FootIKTraceDistance = 100.0f;

    /** Foot IK interpolation speed */
    UPROPERTY(EditDefaultsOnly, Category = "Foot IK")
    float FootIKInterpSpeed = 15.0f;

    /** Speed threshold for walk/run transition */
    UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
    float WalkSpeedThreshold = 180.0f;

    /** Speed threshold for run/sprint transition */
    UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
    float RunSpeedThreshold = 450.0f;

private:
    /** Previous frame locomotion state for transition detection */
    EAnim_LocomotionState PreviousLocomotionState = EAnim_LocomotionState::Idle;

    /** Landing timer to hold land state briefly */
    float LandingTimer = 0.0f;

    /** Was in air last frame? */
    bool bWasInAir = false;
};
