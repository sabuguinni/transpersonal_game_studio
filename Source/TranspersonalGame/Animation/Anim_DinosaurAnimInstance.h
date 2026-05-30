#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Anim_DinosaurAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Alert       UMETA(DisplayName = "Alert"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurMovement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float TurnRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsTurning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_DinosaurState CurrentState = EAnim_DinosaurState::Idle;

    FAnim_DinosaurMovement()
    {
        Speed = 0.0f;
        TurnRate = 0.0f;
        bIsMoving = false;
        bIsTurning = false;
        CurrentState = EAnim_DinosaurState::Idle;
    }
};

USTRUCT(BlueprintType)
struct FAnim_DinosaurBehavior
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float AggressionLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float AlertnessLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bIsHunting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bIsFeeding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    bool bIsResting = false;

    FAnim_DinosaurBehavior()
    {
        AggressionLevel = 0.0f;
        AlertnessLevel = 0.0f;
        HealthPercentage = 1.0f;
        bIsHunting = false;
        bIsFeeding = false;
        bIsResting = false;
    }
};

/**
 * Animation Instance for dinosaur characters
 * Handles species-specific animations and behavioral states
 * Supports carnivore and herbivore animation patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_DinosaurAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_DinosaurAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Movement and Behavior Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_DinosaurMovement MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    FAnim_DinosaurBehavior BehaviorData;

    // Owner Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    APawn* OwnerPawn = nullptr;

    // Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float TurnRateThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float StateTransitionSpeed = 5.0f;

    // Species-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species", meta = (AllowPrivateAccess = "true"))
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species", meta = (AllowPrivateAccess = "true"))
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species", meta = (AllowPrivateAccess = "true"))
    float SpeciesSize = 1.0f; // 0.5 = small, 1.0 = medium, 2.0 = large

    // Animation Montages
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* RoarMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* DeathMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montages", meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* EatMontage = nullptr;

public:
    // Animation Control Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRoarAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayDeathAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayEatAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetDinosaurState(EAnim_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAggressionLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAlertness(float NewLevel);

private:
    void UpdateMovementValues(float DeltaTime);
    void UpdateBehaviorValues(float DeltaTime);
    void UpdateAnimationState(float DeltaTime);
    EAnim_DinosaurState DetermineStateFromMovement();

    // Smoothing variables
    float LastSpeed = 0.0f;
    float LastTurnRate = 0.0f;
    EAnim_DinosaurState LastState = EAnim_DinosaurState::Idle;
    float StateTransitionAlpha = 0.0f;
};