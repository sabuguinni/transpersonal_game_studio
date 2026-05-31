#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

/**
 * Animation Instance for TranspersonalCharacter
 * Handles state-based animations: Idle, Walk, Run, Jump, Fall
 * Uses realistic movement parameters for primitive human survivor
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation state variables
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsFalling;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsJumping;

    // Survival state animations
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthPercent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaPercent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsInjured;

    // Character reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* Character;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* CharacterMovement;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpVelocityThreshold;

public:
    // Animation update functions
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Animation state functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementValues();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalValues();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCharacterState();

    // Animation event functions
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnLanded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnJumped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnStartRunning();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnStopRunning();

    // Survival animation events
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Events")
    void OnHealthCritical();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Events")
    void OnStaminaExhausted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Events")
    void OnFearReaction();

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeed() const { return Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetDirection() const { return Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInAir() const { return bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsRunning() const { return bIsRunning; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsJumping() const { return bIsJumping; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetHealthPercent() const { return HealthPercent; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetStaminaPercent() const { return StaminaPercent; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetFearLevel() const { return FearLevel; }
};