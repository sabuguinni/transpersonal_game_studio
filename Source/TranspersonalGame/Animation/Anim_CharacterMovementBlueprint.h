#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "SharedTypes.h"
#include "Anim_CharacterMovementBlueprint.generated.h"

/**
 * Animation Blueprint for primitive human character movement
 * Handles state transitions between idle, walk, run, jump, and combat states
 * Designed for prehistoric survival gameplay with realistic movement patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterMovementBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterMovementBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Movement State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsIdle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsWalking;

    // Combat State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsDodging;

    // Survival State Variables
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsHungry;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsThirsty;

    // Animation Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleThreshold;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* MovementComponent;

private:
    void UpdateMovementVariables();
    void UpdateCombatVariables();
    void UpdateSurvivalVariables();
    void CalculateDirection();
    void DetermineMovementState();

    // Internal state tracking
    FVector LastFrameVelocity;
    float AccelerationTimer;
    float IdleTimer;
};