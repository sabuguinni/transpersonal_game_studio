#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation states for the prehistoric survivor character.
 * Controls locomotion blend tree transitions.
 */
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Swim        UMETA(DisplayName = "Swim")
};

/**
 * Jump/fall phase for the survivor.
 */
UENUM(BlueprintType)
enum class EAnim_AirState : uint8
{
    Grounded    UMETA(DisplayName = "Grounded"),
    JumpStart   UMETA(DisplayName = "JumpStart"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

/**
 * Combat stance for the survivor.
 */
UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Club        UMETA(DisplayName = "Club")
};

/**
 * DinoSurvivorAnimInstance
 * 
 * Main AnimInstance for the prehistoric human survivor character.
 * Drives locomotion blend space, IK foot placement, combat stances,
 * and survival state reactions (fear, exhaustion, injury).
 * 
 * Architecture:
 *   - NativeUpdateAnimation() dispatches to 5 subsystems each frame
 *   - Locomotion: speed-based blend space (idle→walk→run→sprint)
 *   - Air state: jump/fall/land phase tracking
 *   - IK: foot placement on uneven terrain (two-bone IK)
 *   - Combat: weapon stance overlay
 *   - Survival: lean/sway modifiers from stamina/fear stats
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    // --- UAnimInstance Interface ---
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // =========================================================
    // LOCOMOTION PROPERTIES (read by AnimBP blend space)
    // =========================================================

    /** Current ground speed (cm/s). Drives locomotion blend space X axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed;

    /** Lateral direction (-1 left, 0 forward, +1 right). Drives blend space Y axis. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float LateralDirection;

    /** True when character has velocity > 10 cm/s. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    /** True when character is in the air. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** True when character is crouching. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    /** True when character is sprinting (speed > RunThreshold). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    /** Current locomotion state enum for state machine transitions. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    /** Current air state enum. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_AirState AirState;

    // =========================================================
    // SPEED THRESHOLDS (configurable per character)
    // =========================================================

    /** Speed threshold to transition from idle to walk (cm/s). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Thresholds")
    float WalkThreshold;

    /** Speed threshold to transition from walk to run (cm/s). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Thresholds")
    float RunThreshold;

    /** Speed threshold to transition from run to sprint (cm/s). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Thresholds")
    float SprintThreshold;

    // =========================================================
    // IK FOOT PLACEMENT
    // =========================================================

    /** Left foot IK target offset (Z) for terrain adaptation. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float LeftFootIKOffset;

    /** Right foot IK target offset (Z) for terrain adaptation. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float RightFootIKOffset;

    /** Pelvis offset to keep body centred between foot IK targets. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float PelvisIKOffset;

    /** IK alpha — 0 when in air, 1 when fully grounded. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float IKAlpha;

    // =========================================================
    // COMBAT STANCE
    // =========================================================

    /** Current weapon/combat stance for upper-body overlay. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance CombatStance;

    /** True when character is actively attacking. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    // =========================================================
    // SURVIVAL STATE MODIFIERS
    // =========================================================

    /** Normalised stamina [0..1]. Low stamina adds exhaustion sway. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaNormalised;

    /** Normalised fear [0..1]. High fear adds tremor/crouch lean. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float FearNormalised;

    /** Normalised health [0..1]. Low health adds injury limp. */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float HealthNormalised;

    /** Additive lean angle from exhaustion/fear (degrees). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float SurvivalLeanAngle;

    // =========================================================
    // AIM OFFSET
    // =========================================================

    /** Aim pitch for upper-body aim offset (-90..90). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim", meta = (AllowPrivateAccess = "true"))
    float AimPitch;

    /** Aim yaw for upper-body aim offset (-90..90). */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim", meta = (AllowPrivateAccess = "true"))
    float AimYaw;

private:
    // =========================================================
    // CACHED REFERENCES
    // =========================================================

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // =========================================================
    // SUBSYSTEM UPDATE METHODS
    // =========================================================

    /** Updates GroundSpeed, LateralDirection, bIsMoving, bIsInAir, bIsCrouching. */
    void UpdateLocomotionState(float DeltaSeconds);

    /** Updates LeftFootIKOffset, RightFootIKOffset, PelvisIKOffset, IKAlpha. */
    void UpdateFootIK(float DeltaSeconds);

    /** Updates CombatStance, bIsAttacking. */
    void UpdateCombatState(float DeltaSeconds);

    /** Updates StaminaNormalised, FearNormalised, HealthNormalised, SurvivalLeanAngle. */
    void UpdateSurvivalModifiers(float DeltaSeconds);

    /** Updates AimPitch, AimYaw from controller rotation delta. */
    void UpdateAimOffset(float DeltaSeconds);

    // =========================================================
    // IK HELPERS
    // =========================================================

    /** Performs a line trace downward from foot socket to find terrain Z. */
    float TraceFootIK(FName SocketName, float TraceDistance = 100.0f) const;

    /** Smoothly interpolates a float value toward target at given speed. */
    float InterpToTarget(float Current, float Target, float DeltaSeconds, float Speed) const;

    // =========================================================
    // INTERNAL STATE
    // =========================================================

    /** Vertical velocity at last frame — used to detect landing. */
    float PreviousVerticalVelocity;

    /** Time spent in air — used for landing animation timing. */
    float TimeInAir;

    /** Smoothed IK offsets (interpolated, not raw trace). */
    float SmoothedLeftFootIK;
    float SmoothedRightFootIK;
    float SmoothedPelvisIK;
};
