#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// Forward declarations
class UCharacterMovementComponent;
class ACharacter;

/**
 * Animation state enum for the player character locomotion state machine.
 * Drives blend space selection and montage slot routing.
 */
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    InAir       UMETA(DisplayName = "InAir"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Dead        UMETA(DisplayName = "Dead")
};

/**
 * Montage slot identifiers for the animation montage system.
 */
UENUM(BlueprintType)
enum class EAnim_MontageSlot : uint8
{
    DefaultSlot     UMETA(DisplayName = "DefaultSlot"),
    UpperBody       UMETA(DisplayName = "UpperBody"),
    FullBody        UMETA(DisplayName = "FullBody"),
    Additive        UMETA(DisplayName = "Additive")
};

/**
 * Foot IK data passed from FootIKComponent to the AnimInstance each tick.
 */
USTRUCT(BlueprintType)
struct FAnim_FootIKState
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
    bool bIsOnUnevenTerrain = false;
};

/**
 * UPlayerAnimInstance — the core Animation Instance for the TranspersonalCharacter.
 *
 * Drives a locomotion state machine (Idle/Walk/Run/Sprint/InAir/Landing/Crouching/Dead),
 * exposes speed/direction/lean values for blend spaces, integrates FootIK data,
 * and provides Blueprint-callable montage helpers.
 *
 * Architecture:
 *   - NativeInitializeAnimation(): caches character + movement component refs
 *   - NativeUpdateAnimation(DeltaSeconds): updates all anim properties each tick
 *   - Blueprint graph reads UPROPERTY values to drive blend spaces and state machines
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion Properties ───────────────────────────────────────────────

    /** Current movement speed (cm/s). Drives 1D locomotion blend space. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f;

    /** Lateral movement direction (-180 to 180). Drives strafe blend. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Direction = 0.0f;

    /** Lean amount for banking into turns. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float LeanAmount = 0.0f;

    /** True when the character is in the air (jumping or falling). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    /** True when the character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    /** True when the character is sprinting (speed > SprintThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    /** True when the character is dead. Locks to death pose. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

    /** Current locomotion state for state machine transitions. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ─── Vertical / Air Properties ───────────────────────────────────────────

    /** Vertical velocity (positive = rising, negative = falling). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air",
              meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity = 0.0f;

    /** Time spent in air (seconds). Used for landing impact intensity. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air",
              meta = (AllowPrivateAccess = "true"))
    float TimeInAir = 0.0f;

    // ─── Foot IK Properties ──────────────────────────────────────────────────

    /** Current foot IK state from the FootIKComponent. */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKState FootIKState;

    /** Whether foot IK is currently active (disabled in air). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    bool bFootIKActive = false;

    // ─── Aim Offset Properties ───────────────────────────────────────────────

    /** Aim pitch for upper-body aim offset (-90 to 90). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
              meta = (AllowPrivateAccess = "true"))
    float AimPitch = 0.0f;

    /** Aim yaw for upper-body aim offset (-90 to 90). */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
              meta = (AllowPrivateAccess = "true"))
    float AimYaw = 0.0f;

    // ─── Thresholds (configurable in Blueprint) ──────────────────────────────

    /** Speed threshold to transition from Idle to Walk. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Thresholds")
    float WalkThreshold = 10.0f;

    /** Speed threshold to transition from Walk to Run. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Thresholds")
    float RunThreshold = 200.0f;

    /** Speed threshold to transition from Run to Sprint. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Thresholds")
    float SprintThreshold = 500.0f;

    /** Lean interpolation speed. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Thresholds")
    float LeanInterpSpeed = 4.0f;

    // ─── Blueprint-callable helpers ──────────────────────────────────────────

    /** Returns true if currently playing a montage in the given slot. */
    UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
    bool IsPlayingMontageInSlot(EAnim_MontageSlot Slot) const;

    /** Returns the current locomotion state as a string (for debug HUD). */
    UFUNCTION(BlueprintCallable, Category = "Animation|Debug")
    FString GetLocomotionStateString() const;

    /** Manually set foot IK state (called by FootIKComponent each tick). */
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetFootIKState(const FAnim_FootIKState& NewState);

private:
    /** Cached reference to the owning character. */
    UPROPERTY()
    ACharacter* OwnerCharacter = nullptr;

    /** Cached reference to the character movement component. */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent = nullptr;

    /** Previous speed for lean calculation. */
    float PreviousSpeed = 0.0f;

    /** Previous yaw for lean calculation. */
    float PreviousYaw = 0.0f;

    /** Internal: update locomotion state from current speed/air state. */
    void UpdateLocomotionState();

    /** Internal: update aim offset from controller rotation. */
    void UpdateAimOffset();

    /** Internal: update lean from yaw delta. */
    void UpdateLean(float DeltaSeconds);
};
