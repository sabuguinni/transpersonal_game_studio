#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// Animation state enum for the state machine
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death")
};

// Foot IK data per foot
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float FootAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float HipOffset;

    FAnim_FootIKData()
        : FootLocation(FVector::ZeroVector)
        , FootRotation(FRotator::ZeroRotator)
        , FootAlpha(0.0f)
        , HipOffset(0.0f)
    {}
};

// Locomotion blend parameters
USTRUCT(BlueprintType)
struct FAnim_LocomotionParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAmount;

    FAnim_LocomotionParams()
        : Speed(0.0f)
        , Direction(0.0f)
        , bIsInAir(false)
        , bIsCrouching(false)
        , bIsAccelerating(false)
        , LeanAmount(0.0f)
    {}
};

/**
 * UPlayerAnimInstance
 * Main Animation Instance for the TranspersonalCharacter player.
 * Drives locomotion state machine, foot IK, and upper body overlays.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativePostEvaluateAnimation() override;

    // ---- Locomotion State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionParams LocomotionParams;

    // ---- Blend Space Values ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float BlendSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace", meta = (AllowPrivateAccess = "true"))
    float BlendDirection;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    // ---- Air State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air", meta = (AllowPrivateAccess = "true"))
    float JumpVelocityZ;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Air", meta = (AllowPrivateAccess = "true"))
    float FallTime;

    // ---- Survival State (drives animation overlays) ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float HealthRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    // ---- Blueprint callable helpers ----
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const { return LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnableFootIK() const;

protected:
    // Internal update helpers
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateBlendSpaceValues(float DeltaSeconds);
    void UpdateSurvivalState(float DeltaSeconds);
    void PerformFootTrace(const FName& FootSocketName, FAnim_FootIKData& OutIKData, float DeltaSeconds);

    // Speed thresholds
    static constexpr float WalkSpeedThreshold = 10.0f;
    static constexpr float RunSpeedThreshold  = 300.0f;

    // IK trace settings
    static constexpr float FootTraceLength    = 80.0f;
    static constexpr float FootIKInterpSpeed  = 15.0f;

    // Cached owner reference
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    float LandingTimer;
    bool  bWasInAir;
};
