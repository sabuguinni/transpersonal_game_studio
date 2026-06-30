#pragma once

// PlayerAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260630_002
// UAnimInstance subclass for the prehistoric survivor player character.
// Drives locomotion state machine, IK foot placement, survival state blending.

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerAnimInstance.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Sneak       UMETA(DisplayName = "Sneak"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion Properties ────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSneaking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsClimbing;

    // ─── State Machine ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|StateMachine")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|StateMachine")
    EAnim_LocomotionState PreviousLocomotionState;

    // ─── Blend Space Weights ──────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float SpeedBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float DirectionBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Blend")
    float AimYaw;

    // ─── Survival State Properties ────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Survival")
    float StaminaLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Survival")
    float HungerLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|Survival")
    float ThirstLevel;

    // ─── Foot IK Properties ───────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    bool bEnableFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator RightFootIKRotation;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|FootIK")
    float IKTraceDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|FootIK")
    float IKInterpSpeed;

    // ─── Blueprint-Callable Functions ─────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Anim|Survival")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Anim|Survival")
    void SetStaminaLevel(float NewStaminaLevel);

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetSneaking(bool bNewSneaking);

    UFUNCTION(BlueprintCallable, Category = "Anim|Locomotion")
    void SetClimbing(bool bNewClimbing);

    UFUNCTION(BlueprintPure, Category = "Anim|StateMachine")
    EAnim_LocomotionState GetLocomotionState() const;

    UFUNCTION(BlueprintPure, Category = "Anim|StateMachine")
    bool WasLocomotionStateChangedThisFrame() const;

private:
    // ─── Internal Update Methods ──────────────────────────────────────────

    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAimData();
    void UpdateSurvivalStates();
    void UpdateLocomotionState();
    void UpdateLean(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);

    FVector TraceFootIK(const FVector& FootWorldLocation, FRotator& OutFootRotation);

    // ─── Cached References ────────────────────────────────────────────────

    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* OwnerMovement;

    FVector LastVelocity;
    FVector LeanVelocity;
};
