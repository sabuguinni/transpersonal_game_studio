#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// Forward declarations
class ACharacter;
class UCharacterMovementComponent;

/**
 * Animation states for the prehistoric survivor character
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
    Death       UMETA(DisplayName = "Death"),
};

/**
 * Stance states for combat and survival
 */
UENUM(BlueprintType)
enum class EAnim_StanceState : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Combat      UMETA(DisplayName = "Combat"),
    Stealth     UMETA(DisplayName = "Stealth"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
};

/**
 * TranspersonalAnimInstance
 * 
 * Animation instance for the prehistoric survivor character.
 * Drives locomotion blending, IK foot placement, and survival state animations.
 * Designed for Motion Matching-style fluid transitions between movement states.
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

    // ---- LOCOMOTION PROPERTIES ----

    /** Current movement speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    /** Lateral movement speed for strafing */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LateralSpeed;

    /** Direction of movement relative to character facing (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection;

    /** Is the character in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    /** Is the character moving */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    /** Is the character crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    /** Is the character sprinting */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    /** Vertical velocity for jump/fall blending */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity;

    /** Current locomotion state */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    /** Current stance state */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Stance")
    EAnim_StanceState StanceState;

    // ---- SURVIVAL STATE PROPERTIES ----

    /** Character health (0-100) — affects animation posture */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Health;

    /** Character stamina (0-100) — affects run/sprint availability */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Stamina;

    /** Fear level (0-100) — affects movement jitter and breathing */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    /** Is the character in a low-health state (triggers injured animations) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    /** Is the character exhausted (stamina < 15) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    // ---- IK FOOT PLACEMENT ----

    /** Left foot IK target location */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Left foot IK rotation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator LeftFootIKRotation;

    /** Right foot IK rotation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator RightFootIKRotation;

    /** Pelvis offset for IK (vertical adjustment) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    /** Is foot IK active (disabled in air) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bIsFootIKActive;

    // ---- AIM OFFSET ----

    /** Aim pitch for upper body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    /** Aim yaw for upper body aim offset (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

    // ---- BLEND WEIGHTS ----

    /** Blend weight for additive breathing animation */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float BreathingBlendWeight;

    /** Blend weight for fear tremor additive */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float FearTremorBlendWeight;

    // ---- CONFIGURATION ----

    /** Speed threshold for walk→run transition */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float WalkRunThreshold;

    /** Speed threshold for run→sprint transition */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float RunSprintThreshold;

    /** IK trace distance below foot */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float FootIKTraceDistance;

    /** Smoothing speed for IK transitions */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Config")
    float IKSmoothingSpeed;

    // ---- BLUEPRINT CALLABLE FUNCTIONS ----

    /** Called when character lands after a jump/fall */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void OnLanded();

    /** Force a specific locomotion state (used by game systems) */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    /** Force a specific stance state */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetStanceState(EAnim_StanceState NewState);

    /** Get the blend space position for locomotion (X=speed, Y=direction) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation")
    FVector2D GetLocomotionBlendSpacePosition() const;

private:
    /** Cached reference to owning character */
    UPROPERTY()
    ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    /** Previous frame speed for acceleration detection */
    float PreviousSpeed;

    /** Time since last landed (for landing blend) */
    float TimeSinceLanded;

    /** Is currently in landing animation */
    bool bIsLanding;

    // ---- INTERNAL UPDATE METHODS ----
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateSurvivalData(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateBlendWeights(float DeltaSeconds);
    void DetermineLocomotionState();
    void DetermineStanceState();
    bool TraceFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);
};
