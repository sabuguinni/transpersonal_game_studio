#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_TribalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Fighting    UMETA(DisplayName = "Fighting"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Injured     UMETA(DisplayName = "Injured"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_TribalWeapon : uint8
{
    None        UMETA(DisplayName = "None"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    StoneAxe    UMETA(DisplayName = "Stone Axe"),
    Knife       UMETA(DisplayName = "Knife")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalMovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement") 
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    FAnim_TribalMovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        bIsCrouching = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalCombatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_TribalWeapon CurrentWeapon;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AttackSpeed;

    FAnim_TribalCombatData()
    {
        bIsAttacking = false;
        bIsBlocking = false;
        bIsAiming = false;
        CurrentWeapon = EAnim_TribalWeapon::None;
        AttackSpeed = 1.0f;
    }
};

/**
 * Advanced animation instance for tribal characters in the Cretaceous survival game.
 * Handles complex state transitions, weapon-based animations, and survival activities.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TribalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_TribalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_TribalState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_TribalState PreviousState;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FAnim_TribalMovementData MovementData;

    // Combat Data
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FAnim_TribalCombatData CombatData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    // Animation Blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float IdleToWalkBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float WalkToRunBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float CombatBlend;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float SurvivalActivityBlend;

    // Survival Activities
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsGathering;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsCrafting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHunting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float GatheringProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float CraftingProgress;

    // Health and Status
    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsFatigued;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float FatigueLevel;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetTribalState(EAnim_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetWeapon(EAnim_TribalWeapon NewWeapon);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerGatherAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerCraftAnimation();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayMovementAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayCombatAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlaySurvivalAnimation() const;

protected:
    // Internal update functions
    void UpdateMovementData();
    void UpdateCombatData();
    void UpdateSurvivalData();
    void UpdateHealthData();
    void UpdateAnimationBlends();
    void UpdateStateTransitions();

    // State transition logic
    bool CanTransitionTo(EAnim_TribalState NewState) const;
    void HandleStateTransition(EAnim_TribalState NewState);

    // Animation timing
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Timing")
    float StateTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Timing")
    float BlendUpdateRate;

    // Speed thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Thresholds")
    float MovementStopThreshold;

private:
    float TimeSinceLastStateChange;
    float LastUpdateTime;
};