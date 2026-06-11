#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimBlueprint.h"
#include "Anim_MetaHumanController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MetaHumanState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Combat      UMETA(DisplayName = "Combat"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Injured     UMETA(DisplayName = "Injured")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MetaHumanParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement") 
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EAnim_MetaHumanState CurrentState = EAnim_MetaHumanState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthPercent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaPercent = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableLookAtIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bIsInteracting = false;
};

/**
 * MetaHuman Animation Controller
 * Manages complex animation states for MetaHuman characters
 * Integrates with survival systems and environmental adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MetaHumanController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MetaHumanController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Parameters")
    FAnim_MetaHumanParams AnimParams;

    // Animation Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> CombatMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> InteractionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> SurvivalMontage;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float LookAtIKInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FVector LeftFootIKOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FVector RightFootIKOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FRotator LookAtRotation = FRotator::ZeroRotator;

public:
    // Animation State Management
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetAnimationState(EAnim_MetaHumanState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateMovementParams(float InSpeed, float InDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void UpdateSurvivalParams(float Health, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetCombatMode(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetInteractionMode(bool bInteracting);

    // IK Control
    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void UpdateLookAtIK(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void EnableLookAtIK(bool bEnable);

    // Animation Montage Control
    UFUNCTION(BlueprintCallable, Category = "Montage Control")
    void PlayIdleVariation(int32 VariationIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Montage Control")
    void PlayCombatAction(int32 ActionIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Montage Control")
    void PlayInteractionAction(int32 ActionIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Montage Control")
    void PlaySurvivalAction(int32 ActionIndex = 0);

protected:
    // Internal state tracking
    EAnim_MetaHumanState PreviousState = EAnim_MetaHumanState::Idle;
    float StateTransitionTime = 0.0f;
    bool bIsTransitioning = false;

    // IK calculation helpers
    FVector CalculateFootIKOffset(const FName& SocketName);
    FRotator CalculateLookAtRotation(const FVector& TargetLocation);
    void InterpolateIKValues(float DeltaTime);

    // Animation blending helpers
    float CalculateSpeedBlendAlpha() const;
    float CalculateDirectionBlendAlpha() const;
    float CalculateSurvivalBlendAlpha() const;
};