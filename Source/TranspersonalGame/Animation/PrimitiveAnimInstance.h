#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimInstance.generated.h"

/**
 * Animation Blueprint instance for primitive characters
 * Handles real-time animation state updates and blending
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrimitiveAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrimitiveAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Animation state variables exposed to Blueprint
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement State")
    EAnim_MovementState MovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    EAnim_CombatState CombatState;

    // Movement parameters for blend spaces
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    float Velocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Parameters")
    bool bIsMoving;

    // Combat parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    bool bIsAttacking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    bool bIsBlocking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    bool bHasWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    int32 ComboIndex;

    // Fear and survival parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Parameters")
    float FearLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Parameters")
    float HealthPercentage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Parameters")
    float StaminaPercentage;

    // Environmental adaptation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Parameters")
    bool bIsOnUnevenTerrain;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Parameters")
    float GroundSlope;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment Parameters")
    bool bIsInWater;

    // Animation event functions
    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnLanding();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnAttackStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnAttackEnd();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnFearReaction();

protected:
    // Character references
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TObjectPtr<UPrimitiveAnimationController> AnimController;

private:
    // Internal update functions
    void UpdateMovementParameters();
    void UpdateCombatParameters();
    void UpdateSurvivalParameters();
    void UpdateEnvironmentParameters();

    // State transition helpers
    bool ShouldTransitionToIdle() const;
    bool ShouldTransitionToWalk() const;
    bool ShouldTransitionToRun() const;
    bool ShouldTransitionToJump() const;

    // Smoothing parameters
    float SpeedSmoothingRate = 5.0f;
    float DirectionSmoothingRate = 10.0f;
    
    // Previous frame values for smoothing
    float PreviousSpeed;
    float PreviousDirection;
};