#pragma once

// DinosaurAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// UAnimInstance subclass for all dinosaur species
// Drives locomotion blendspace, combat montages, foot IK, procedural tail/head

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAnim_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurAttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    TailSwipe       UMETA(DisplayName = "Tail Swipe"),
    Charge          UMETA(DisplayName = "Charge"),
    Stomp           UMETA(DisplayName = "Stomp"),
    HornThrust      UMETA(DisplayName = "Horn Thrust")
};

UENUM(BlueprintType)
enum class EAnim_DinosaurLocomotionState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walk            UMETA(DisplayName = "Walk"),
    Run             UMETA(DisplayName = "Run"),
    Sprint          UMETA(DisplayName = "Sprint"),
    Swim            UMETA(DisplayName = "Swim"),
    Fly             UMETA(DisplayName = "Fly")
};

// ─────────────────────────────────────────────────────────────────────────────
// MAIN CLASS
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinosaurAnimInstance();

    // ── UAnimInstance overrides ──────────────────────────────────────────────
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_DinosaurLocomotionState LocomotionState;

    // ── Combat ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsRoaring;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsEating;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsStunned;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    EAnim_DinosaurAttackType CurrentAttackType;

    // ── Behavior ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behavior")
    bool bIsAlerted;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behavior")
    bool bIsSearching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behavior")
    bool bIsSleeping;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behavior")
    bool bIsIdle;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behavior")
    int32 IdleVariantIndex;

    // ── Blend Weights ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AttackBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float RoarBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float InjuryBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AlertBlendWeight;

    // ── Species & Stats ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Species")
    EAnim_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Stats")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Stats")
    float StaminaPercent;

    // ── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKOffset;

    // ── Procedural ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Procedural")
    float TailSwayAngle;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Procedural")
    float HeadLookAtAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Procedural")
    FVector HeadLookAtTarget;

    // ── Public API (callable from AI/Behavior Trees) ─────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerAttack(EAnim_DinosaurAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerRoar();

    UFUNCTION(BlueprintCallable, Category = "Anim|Behavior")
    void SetAlertState(bool bAlerted, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Anim|Stats")
    void SetHealthPercent(float NewHealthPercent);

    UFUNCTION(BlueprintCallable, Category = "Anim|Stats")
    void SetStaminaPercent(float NewStaminaPercent);

private:
    // ── Cached references ────────────────────────────────────────────────────
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovement;

    // ── Internal state ───────────────────────────────────────────────────────
    float TimeSinceLastAttack;
    float TimeSinceLastRoar;
    float IdleVariantTimer;

    // ── Update subsystems ────────────────────────────────────────────────────
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateCombatState(float DeltaSeconds);
    void UpdateBehaviorState(float DeltaSeconds);
    void UpdateProceduralElements(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);

    // ── Species helpers ──────────────────────────────────────────────────────
    float GetRunThresholdForSpecies() const;
    float GetFootSpreadForSpecies() const;
    float GetFootForwardOffsetForSpecies() const;
    float GetIdleVariantDuration() const;
};
