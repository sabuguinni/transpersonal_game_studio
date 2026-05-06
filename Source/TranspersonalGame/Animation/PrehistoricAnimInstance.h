#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "PrehistoricAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_ActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Combat      UMETA(DisplayName = "Combat"),
    Eating      UMETA(DisplayName = "Eating"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Building    UMETA(DisplayName = "Building"),
    Hunting     UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Temperature = 20.0f;

    FAnim_SurvivalData()
    {
        Health = 100.0f;
        Stamina = 100.0f;
        Hunger = 0.0f;
        Thirst = 0.0f;
        Fear = 0.0f;
        Temperature = 20.0f;
    }
};

/**
 * Animation Instance for prehistoric characters with survival mechanics
 * Handles movement states, action states, and survival-based animation blending
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State Machine
    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_MovementState CurrentMovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_ActionState CurrentActionState;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    // Survival Data
    UPROPERTY(BlueprintReadOnly, Category = "Survival Data")
    FAnim_SurvivalData SurvivalData;

    // Animation Blending Parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float WalkRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float FearIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float FatigueLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float InjuryLevel = 0.0f;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

private:
    // Update Functions
    void UpdateMovementData();
    void UpdateSurvivalData();
    void UpdateMovementState();
    void UpdateActionState();
    void UpdateBlendingParameters();

    // State Transition Logic
    bool ShouldTransitionToRunning() const;
    bool ShouldTransitionToWalking() const;
    bool ShouldTransitionToIdle() const;
    bool ShouldTransitionToJumping() const;
    bool ShouldTransitionToFalling() const;

    // Animation Triggers
    UFUNCTION(BlueprintCallable, Category = "Animation Triggers")
    void TriggerGatheringAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Triggers")
    void TriggerCraftingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Triggers")
    void TriggerCombatAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Triggers")
    void TriggerEatingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation Triggers")
    void TriggerDrinkingAnimation();

public:
    // Blueprint Interface
    UFUNCTION(BlueprintCallable, Category = "Animation Control")
    void SetActionState(EAnim_ActionState NewActionState);

    UFUNCTION(BlueprintPure, Category = "Animation Control")
    bool IsPerformingAction() const;

    UFUNCTION(BlueprintPure, Category = "Animation Control")
    float GetMovementSpeedRatio() const;

    UFUNCTION(BlueprintPure, Category = "Animation Control")
    bool ShouldPlayFearAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation Control")
    bool ShouldPlayFatigueAnimation() const;
};