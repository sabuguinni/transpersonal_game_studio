#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

/**
 * UDinosaurAnimInstance
 * Animation instance for all dinosaur species in the prehistoric survival game.
 * Drives locomotion blending, attack montages, alert states, and death poses.
 * Works with the Behavior Tree / NPC system to reflect AI state in animation.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion ───────────────────────────────────────────────────────────

    /** Current ground speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Smoothed speed for blend space — avoids jitter on direction changes */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    /** Forward/backward direction scalar (-1 to 1) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float ForwardSpeed;

    /** Lateral direction scalar (-1 to 1) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LateralSpeed;

    /** True when the dinosaur is airborne (e.g. jumping raptors) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when the dinosaur is actively accelerating */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAccelerating;

    /** True when moving at maximum sprint speed */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** Locomotion speed as 0-1 alpha for blend spaces (0=idle, 0.5=walk, 1=run) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LocomotionAlpha;

    // ─── Combat & Behaviour ───────────────────────────────────────────────────

    /** True when the dinosaur is in an active attack state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** True when the dinosaur has detected a threat and is alert */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAlert;

    /** True when the dinosaur is actively chasing a target */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsChasing;

    /** True when the dinosaur is performing a roar/display behaviour */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsRoaring;

    /** True when the dinosaur is feeding on a carcass */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsFeeding;

    /** True when the dinosaur is in a death state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsDead;

    /** Normalised health (0-1) — drives injury limping blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float HealthAlpha;

    /** Injury severity (0=healthy, 1=critically injured) — drives limp pose */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float InjuryAlpha;

    // ─── IK & Foot Placement ──────────────────────────────────────────────────

    /** Enable foot IK for terrain adaptation */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    bool bEnableFootIK;

    /** Left front foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFrontFootIKTarget;

    /** Right front foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFrontFootIKTarget;

    /** Left rear foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftRearFootIKTarget;

    /** Right rear foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightRearFootIKTarget;

    /** Body lean alpha for slope adaptation (0=flat, 1=steep) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float BodyLeanAlpha;

    // ─── Head Tracking ────────────────────────────────────────────────────────

    /** True when the dinosaur is tracking a target with its head */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|HeadTracking")
    bool bIsHeadTracking;

    /** World-space look-at target for head IK */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|HeadTracking")
    FVector HeadLookAtTarget;

    /** Head tracking weight (0=no tracking, 1=full tracking) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|HeadTracking")
    float HeadTrackingWeight;

    // ─── Species Configuration ────────────────────────────────────────────────

    /** Sprint speed threshold (cm/s) — above this = sprinting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float SprintSpeedThreshold;

    /** Walk speed threshold (cm/s) — above this = walking */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    /** Speed smoothing rate (higher = faster response) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float SpeedSmoothingRate;

    /** Number of feet for this species (2=biped, 4=quadruped) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    int32 FootCount;

    // ─── Blueprint Events ─────────────────────────────────────────────────────

    /** Called when the dinosaur transitions from idle to alert */
    UFUNCTION(BlueprintImplementableEvent, Category = "Anim|Events")
    void OnAlertStateEntered();

    /** Called when the dinosaur begins an attack */
    UFUNCTION(BlueprintImplementableEvent, Category = "Anim|Events")
    void OnAttackStarted();

    /** Called when the dinosaur dies */
    UFUNCTION(BlueprintImplementableEvent, Category = "Anim|Events")
    void OnDeathStarted();

    /** Called when the dinosaur begins roaring */
    UFUNCTION(BlueprintImplementableEvent, Category = "Anim|Events")
    void OnRoarStarted();

protected:
    /** Cached owning pawn */
    UPROPERTY()
    APawn* OwnerPawn;

    /** Cached movement component */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    /** Previous bIsAlert state for transition detection */
    bool bWasAlert;

    /** Previous bIsAttacking state for transition detection */
    bool bWasAttacking;

    /** Previous bIsDead state for transition detection */
    bool bWasDead;

    /** Previous bIsRoaring state for transition detection */
    bool bWasRoaring;

    /** Update foot IK targets via line traces */
    void UpdateFootIK();

    /** Update head tracking towards the AI's current focus target */
    void UpdateHeadTracking();

    /** Compute locomotion alpha from speed */
    float ComputeLocomotionAlpha(float CurrentSpeed) const;
};
