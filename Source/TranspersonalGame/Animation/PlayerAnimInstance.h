#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerAnimInstance.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_PlayerLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim")
};

UENUM(BlueprintType)
enum class EAnim_PlayerCombatState : uint8
{
    Unarmed         UMETA(DisplayName = "Unarmed"),
    MeleeReady      UMETA(DisplayName = "MeleeReady"),
    MeleeAttack     UMETA(DisplayName = "MeleeAttack"),
    RangedAim       UMETA(DisplayName = "RangedAim"),
    RangedFire      UMETA(DisplayName = "RangedFire"),
    Block           UMETA(DisplayName = "Block"),
    Stagger         UMETA(DisplayName = "Stagger"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_PlayerSurvivalState : uint8
{
    Healthy         UMETA(DisplayName = "Healthy"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Injured         UMETA(DisplayName = "Injured"),
    Starving        UMETA(DisplayName = "Starving"),
    Dehydrated      UMETA(DisplayName = "Dehydrated"),
    Hypothermic     UMETA(DisplayName = "Hypothermic"),
    Overheated      UMETA(DisplayName = "Overheated")
};

// ============================================================
// STRUCT — foot IK data
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float PelvisOffset = 0.0f;
};

// ============================================================
// STRUCT — lean/aim offset data
// ============================================================

USTRUCT(BlueprintType)
struct FAnim_LeanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanRight = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float LeanForward = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Lean")
    float AimYaw = 0.0f;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // ---- UAnimInstance overrides ----
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeBeginPlay() override;

    // ---- Locomotion ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float VerticalVelocity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSwimming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_PlayerLocomotionState LocomotionState = EAnim_PlayerLocomotionState::Idle;

    // ---- Blend weights ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float WalkRunAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float GroundedAirborneAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float CrouchAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float SprintAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float SurvivalPostureAlpha = 0.0f;

    // ---- Combat ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_PlayerCombatState CombatState = EAnim_PlayerCombatState::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float WeaponDrawAlpha = 0.0f;

    // ---- Survival posture ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    EAnim_PlayerSurvivalState SurvivalState = EAnim_PlayerSurvivalState::Healthy;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float HungerNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ThirstNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearNormalized = 0.0f;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bEnableFootIK = true;

    // ---- Lean / Aim offset ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    FAnim_LeanData LeanData;

    // ---- Additive layers ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float BreathingAlpha = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float ExhaustionAdditive = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Additive")
    float InjuryAdditive = 0.0f;

    // ---- Interaction ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction")
    bool bIsInteracting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction")
    bool bIsCarrying = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Interaction")
    bool bIsCrafting = false;

    // ---- Blueprint callable utilities ----
    UFUNCTION(BlueprintCallable, Category = "Animation|Utilities")
    void SetCombatState(EAnim_PlayerCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation|Utilities")
    void SetSurvivalStats(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Animation|Utilities")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation|Utilities")
    void TriggerLandMontage();

    UFUNCTION(BlueprintPure, Category = "Animation|Utilities")
    float GetSpeedNormalized() const;

    UFUNCTION(BlueprintPure, Category = "Animation|Utilities")
    bool ShouldPlayExhaustionLayer() const;

private:
    // Internal cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    // Internal helpers
    void UpdateLocomotionState();
    void UpdateCombatState();
    void UpdateSurvivalPosture();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateLeanData(float DeltaSeconds);
    void UpdateBlendWeights(float DeltaSeconds);
    void UpdateAdditiveLayerWeights(float DeltaSeconds);

    // Foot IK trace helpers
    bool TraceFootPosition(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation) const;
    void SmoothFootIKTarget(FVector& Current, const FVector& Target, float DeltaSeconds, float InterpSpeed) const;

    // Smoothing state
    FVector SmoothedLeftFoot = FVector::ZeroVector;
    FVector SmoothedRightFoot = FVector::ZeroVector;
    float SmoothedPelvisOffset = 0.0f;
    float SmoothedLeanRight = 0.0f;
    float SmoothedLeanForward = 0.0f;
    FVector PreviousVelocity = FVector::ZeroVector;

    // Config constants
    static constexpr float WalkSpeedThreshold = 10.0f;
    static constexpr float RunSpeedThreshold = 300.0f;
    static constexpr float SprintSpeedThreshold = 550.0f;
    static constexpr float FootIKTraceDistance = 50.0f;
    static constexpr float FootIKInterpSpeed = 15.0f;
    static constexpr float LeanInterpSpeed = 8.0f;
    static constexpr float BlendInterpSpeed = 6.0f;
};
