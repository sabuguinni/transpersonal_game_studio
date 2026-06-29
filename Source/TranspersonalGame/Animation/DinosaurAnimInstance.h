#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

/**
 * Dinosaur locomotion states for blend tree selection.
 * Prefix: EAnim_ to avoid collision with other agent types.
 */
UENUM(BlueprintType)
enum class EAnim_DinoLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Trot        UMETA(DisplayName = "Trot"),
    Run         UMETA(DisplayName = "Run"),
    Attack      UMETA(DisplayName = "Attack"),
    Eat         UMETA(DisplayName = "Eat"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Death       UMETA(DisplayName = "Death"),
    Roar        UMETA(DisplayName = "Roar"),
    Investigate UMETA(DisplayName = "Investigate")
};

/**
 * Dinosaur size category — affects animation playback rate and blend weights.
 */
UENUM(BlueprintType)
enum class EAnim_DinoSizeCategory : uint8
{
    Small   UMETA(DisplayName = "Small"),    // Velociraptor, Compsognathus
    Medium  UMETA(DisplayName = "Medium"),   // Dilophosaurus, Iguanodon
    Large   UMETA(DisplayName = "Large"),    // T-Rex, Allosaurus
    Massive UMETA(DisplayName = "Massive")   // Brachiosaurus, Argentinosaurus
};

/**
 * UDinosaurAnimInstance
 *
 * AnimInstance for all dinosaur species in the prehistoric survival game.
 * Drives locomotion blend spaces, attack montages, and procedural IK for
 * foot placement on uneven terrain. Supports all size categories from
 * small raptors to massive sauropods.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    /** Called once when the AnimInstance is created */
    virtual void NativeInitializeAnimation() override;

    /** Called every frame to update animation variables */
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Current ground speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Smoothed speed for blend space — avoids jitter on uneven terrain */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    /** Movement direction relative to actor forward (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float MovementDirection;

    /** True when the dinosaur is airborne (e.g. jumping raptors) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when velocity is above minimum walk threshold */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    /** True when running at full sprint speed */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** Normalised speed 0..1 for blend space axis */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float NormalisedSpeed;

    // ── State Machine ────────────────────────────────────────────────────────

    /** Current high-level locomotion state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    EAnim_DinoLocomotionState LocomotionState;

    /** Previous state — used for transition logic */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    EAnim_DinoLocomotionState PreviousLocomotionState;

    /** Size category — affects playback rate and IK reach */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State")
    EAnim_DinoSizeCategory SizeCategory;

    // ── Combat ───────────────────────────────────────────────────────────────

    /** True when actively attacking a target */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** True when in alert/threat-detection posture */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAlert;

    /** Normalised health 0..1 — affects posture and limp blend */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float HealthAlpha;

    /** True when health < 0.3 — triggers limp/wounded animations */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsWounded;

    // ── Procedural IK ────────────────────────────────────────────────────────

    /** Left front foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector IK_FrontLeftFoot;

    /** Right front foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector IK_FrontRightFoot;

    /** Left rear foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector IK_RearLeftFoot;

    /** Right rear foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector IK_RearRightFoot;

    /** Body lean alpha for slope adaptation (0=flat, 1=steep) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float SlopeLeanAlpha;

    /** Pelvis vertical offset for IK foot placement */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float PelvisOffset;

    // ── Behaviour ────────────────────────────────────────────────────────────

    /** True when eating/feeding from a carcass */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    bool bIsEating;

    /** True when sleeping or resting */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    bool bIsSleeping;

    /** True when performing a roar/territorial display */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    bool bIsRoaring;

    /** Tail sway oscillation value (-1..1) driven by speed */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    float TailSwayAlpha;

    // ── Playback Rate ─────────────────────────────────────────────────────────

    /** Animation playback rate — larger dinos play slower */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Playback")
    float PlayRate;

    // ── Configuration ─────────────────────────────────────────────────────────

    /** Maximum run speed for this species (cm/s) — set by AI/species data */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float MaxRunSpeed;

    /** Minimum speed to enter walk state (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float WalkSpeedThreshold;

    /** Speed above which the dino is considered sprinting (cm/s) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float SprintSpeedThreshold;

    /** IK trace distance below foot bone (cm) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float IKTraceDistance;

    /** Speed smoothing factor (0=instant, 1=never changes) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float SpeedSmoothingAlpha;

private:
    /** Cached owner pawn */
    UPROPERTY()
    APawn* OwnerPawn;

    /** Cached movement component */
    UPROPERTY()
    UMovementComponent* MovementComp;

    /** Elapsed time for tail sway oscillation */
    float TailSwayTime;

    /** Determines locomotion state from current speed */
    void UpdateLocomotionState();

    /** Performs foot IK traces and updates IK target locations */
    void UpdateFootIK(float DeltaSeconds);

    /** Computes play rate based on size category */
    void UpdatePlayRate();

    /** Computes tail sway oscillation */
    void UpdateTailSway(float DeltaSeconds);
};
