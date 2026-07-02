// TranspersonalAnimInstance.h
// Animation Agent #10 — Transpersonal Game Studio
// Prehistoric survival game character animation instance

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// ============================================================
// ENUMS — declared at global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_CharacterState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Jump        UMETA(DisplayName = "Jump"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death"),
    Wounded     UMETA(DisplayName = "Wounded")
};

UENUM(BlueprintType)
enum class EAnim_AttackType : uint8
{
    LightMelee  UMETA(DisplayName = "LightMelee"),
    HeavyMelee  UMETA(DisplayName = "HeavyMelee"),
    Throw       UMETA(DisplayName = "Throw"),
    Block       UMETA(DisplayName = "Block")
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --------------------------------------------------------
    // LOCOMOTION PROPERTIES
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    // --------------------------------------------------------
    // FOOT IK PROPERTIES
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKInterpSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float LeftFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float RightFootIKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootOffset;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector PelvisOffset;

    // --------------------------------------------------------
    // STATE MACHINE PROPERTIES
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_CharacterState AnimState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_CharacterState PreviousAnimState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    float StateBlendAlpha;

    // --------------------------------------------------------
    // SURVIVAL ANIMATION PROPERTIES
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    bool bIsWounded;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Survival")
    float WoundSeverity;

    // --------------------------------------------------------
    // AIM OFFSET / LEAN
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float LeanAmount;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float AimYaw;

    // --------------------------------------------------------
    // PUBLIC FUNCTIONS
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Animation|Survival")
    void SetSurvivalStats(float Stamina, float Fear, bool bWounded, float WoundLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void TriggerAttackMontage(EAnim_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void TriggerDeathMontage();

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    float GetLocomotionBlendWeight() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    bool ShouldUseSprintAnimation() const;

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal update methods
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAnimationState(float DeltaSeconds);
    void UpdateFootIK(float DeltaSeconds);
    void UpdateSurvivalAnimations(float DeltaSeconds);
    void UpdateLeanAndAim(float DeltaSeconds);
};
