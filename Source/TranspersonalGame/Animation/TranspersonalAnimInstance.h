#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation states for the prehistoric survivor character.
 * Controls blend tree transitions based on movement and survival state.
 */
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Armed       UMETA(DisplayName = "Armed"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Staggered   UMETA(DisplayName = "Staggered")
};

/**
 * TranspersonalAnimInstance
 * 
 * Core animation instance for the prehistoric survivor player character.
 * Drives locomotion blend spaces, IK foot placement, and combat montages.
 * Designed for realistic weight and momentum — each movement reflects the
 * physical state of a human surviving in a dangerous prehistoric world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeBeginPlay() override;

    // ─── Movement State ───────────────────────────────────────────────────────

    /** Current movement state driving the locomotion state machine */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    EAnim_MovementState MovementState;

    /** Current combat posture */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_CombatState CombatState;

    /** Ground speed (XY plane only) used to drive blend spaces */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float GroundSpeed;

    /** Full 3D velocity magnitude including vertical */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float VelocityMagnitude;

    /** Direction of movement relative to character facing (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float MovementDirection;

    /** True when character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsInAir;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsCrouching;

    /** True when character is sprinting (speed > RunThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsSprinting;

    /** True when character is moving (GroundSpeed > MovementThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsMoving;

    // ─── Survival State ───────────────────────────────────────────────────────

    /** Health ratio [0,1] — affects posture and movement quality */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthRatio;

    /** Stamina ratio [0,1] — affects sprint animation and breathing */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio;

    /** Fear level [0,1] — affects idle fidget and movement urgency */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    /** True when character is exhausted (stamina < 0.15) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    // ─── IK Foot Placement ────────────────────────────────────────────────────

    /** Left foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Left foot IK rotation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator LeftFootIKRotation;

    /** Right foot IK rotation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator RightFootIKRotation;

    /** Pelvis offset for foot IK compensation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    /** IK alpha — blends IK on/off based on movement state */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float IKAlpha;

    // ─── Lean & Aim ───────────────────────────────────────────────────────────

    /** Lean amount for banking into turns (-1 to 1) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Lean")
    float LeanAmount;

    /** Aim pitch offset for upper body aiming (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimPitch;

    /** Aim yaw offset for upper body rotation (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimYaw;

    // ─── Thresholds ───────────────────────────────────────────────────────────

    /** Minimum speed to be considered moving */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float MovementThreshold;

    /** Speed threshold to transition from walk to run */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float RunThreshold;

    /** Stamina level below which exhaustion animations play */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float ExhaustionThreshold;

    /** IK trace distance below feet */
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float IKTraceDistance;

    // ─── Public Methods ───────────────────────────────────────────────────────

    /** Update foot IK positions via line traces */
    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void UpdateFootIK(float DeltaSeconds);

    /** Get the current locomotion blend weight for blend space */
    UFUNCTION(BlueprintPure, Category = "Animation|Movement")
    float GetLocomotionBlendWeight() const;

    /** Returns true if a specific montage slot is active */
    UFUNCTION(BlueprintPure, Category = "Animation|Montage")
    bool IsSlotActive(FName SlotName) const;

private:
    /** Cached owning character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Update movement state from velocity and movement component */
    void UpdateMovementState();

    /** Update survival stats from character */
    void UpdateSurvivalState();

    /** Update lean and aim offsets */
    void UpdateLeanAndAim(float DeltaSeconds);

    /** Perform a foot IK trace and return hit result */
    bool TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);

    /** Smoothly interpolate pelvis offset */
    float SmoothedPelvisOffset;

    /** Previous velocity for lean calculation */
    FVector PreviousVelocity;

    /** Lean smoothing speed */
    float LeanSmoothSpeed;
};
