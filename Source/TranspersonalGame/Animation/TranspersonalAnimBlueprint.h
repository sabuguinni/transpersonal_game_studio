// TranspersonalAnimBlueprint.h
// Animation Agent #10 — PROD_CYCLE_AUTO_20260629_002
// Player character AnimInstance — drives idle/walk/run/jump/combat blend states
// for the prehistoric human survivor. Works with ATranspersonalCharacter.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimBlueprint.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_PlayerLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_PlayerCombatState : uint8
{
    Unarmed         UMETA(DisplayName = "Unarmed"),
    MeleeStone      UMETA(DisplayName = "MeleeStone"),
    MeleeSpear      UMETA(DisplayName = "MeleeSpear"),
    RangedThrow     UMETA(DisplayName = "RangedThrow"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Stunned         UMETA(DisplayName = "Stunned")
};

UENUM(BlueprintType)
enum class EAnim_PlayerSurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Injured     UMETA(DisplayName = "Injured"),
    Critical    UMETA(DisplayName = "Critical")
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimBlueprint();

    // ---- UAnimInstance overrides ----
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Locomotion state ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsJumping;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsLanding;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_PlayerLocomotionState LocomotionState;

    // ---- Blend space inputs ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float BlendSpaceX;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float BlendSpaceY;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    float AimYaw;

    // ---- Combat state ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_PlayerCombatState CombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AttackBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float BlockBlendWeight;

    // ---- Survival state ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    EAnim_PlayerSurvivalState SurvivalState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HungerPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float InjuredBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ExhaustedBlendWeight;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bEnableFootIK;

    // ---- Hand IK (tool/weapon holding) ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightHandIKTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float RightHandIKAlpha;

    // ---- Additive overlays ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float BreathingAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float LandingImpactAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float TurnInPlaceAlpha;

    // ---- Blueprint callable setters ----
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void TriggerPlayerAttack(EAnim_PlayerCombatState InCombatState);

    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void OnPlayerAttackEnd();

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetPlayerHealth(float NewHealth, float NewStamina, float NewHunger);

    UFUNCTION(BlueprintCallable, Category = "Animation|Locomotion")
    void SetSprintState(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Animation|IK")
    void SetFootIKEnabled(bool bEnabled);

private:
    // ---- Internal update helpers ----
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateCombat(float DeltaSeconds);
    void UpdateSurvival(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAdditives(float DeltaSeconds);

    // ---- Cached references ----
    UPROPERTY()
    APawn* OwnerPawn;

    UPROPERTY()
    class UCharacterMovementComponent* OwnerMovement;

    // Internal timing
    float TimeSinceLastMove;
    float LandingTimer;
    float BreathingPhase;
};
