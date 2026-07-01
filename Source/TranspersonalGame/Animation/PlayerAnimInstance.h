// Copyright Transpersonal Game Studio. All Rights Reserved.
// PlayerAnimInstance.h — Animation Agent #10
// UAnimInstance subclass for the prehistoric survivor player character.
// Handles locomotion blending, foot IK, weapon state, and survival-driven animation.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
};

UENUM(BlueprintType)
enum class EAnim_WeaponState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Torch       UMETA(DisplayName = "Torch"),
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — must be at global scope (UHT rule)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// MAIN CLASS
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // ── UAnimInstance interface ──────────────────────────────────────────────
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion state ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
        meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity = 0.0f;

    // ── Weapon state ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Weapon",
        meta = (AllowPrivateAccess = "true"))
    EAnim_WeaponState WeaponState = EAnim_WeaponState::Unarmed;

    // ── Survival-driven animation ────────────────────────────────────────────

    /** 0-1: 0 = full health, 1 = near death — drives injury limp */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    float InjuryAlpha = 0.0f;

    /** 0-1: 0 = calm, 1 = maximum fear — drives trembling/panic additive */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    float FearAlpha = 0.0f;

    /** 0-1: 0 = full stamina, 1 = exhausted — drives heavy breathing additive */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
        meta = (AllowPrivateAccess = "true"))
    float ExhaustionAlpha = 0.0f;

    // ── Foot IK ──────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
        meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
        meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
        meta = (AllowPrivateAccess = "true"))
    float PelvisOffset = 0.0f;

    // ── Aim offset ───────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim",
        meta = (AllowPrivateAccess = "true"))
    FRotator AimRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim",
        meta = (AllowPrivateAccess = "true"))
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim",
        meta = (AllowPrivateAccess = "true"))
    float AimYaw = 0.0f;

    // ── Public functions ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetWeaponState(EAnim_WeaponState NewWeaponState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerLandingResponse();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionState; }

private:
    // ── Internal helpers ─────────────────────────────────────────────────────

    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalAnimations(float DeltaSeconds);
    void UpdateAimOffset();
    void SolveFootIK(const FName& FootSocketName, FAnim_FootIKData& OutFootData, float DeltaSeconds);

    // ── Cached references ────────────────────────────────────────────────────

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // ── Internal state ───────────────────────────────────────────────────────

    float LandingTimer = 0.0f;
    bool bLandingActive = false;

    static constexpr float WalkSpeedThreshold  = 10.0f;
    static constexpr float RunSpeedThreshold   = 250.0f;
    static constexpr float SprintSpeedThreshold = 450.0f;
    static constexpr float FootIKTraceDistance  = 75.0f;
    static constexpr float FootIKInterpSpeed    = 15.0f;
    static constexpr float LandingDuration      = 0.35f;
};
