#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_MotionMatchingSystem.h"
#include "Anim_PrimitiveAnimInstance.generated.h"

UCLASS()
class TRANSPERSONALGAME_API UAnim_PrimitiveAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    // IK Properties
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FAnim_IKFootData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKBodyOffset;

    // Survival Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted;

    // Combat Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_WeaponType CurrentWeaponType;

    // Environmental Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float GroundSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsOnUnevenTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    bool bIsInWater;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float WaterDepth;

protected:
    // Component References
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnim_MotionMatchingSystem* MotionMatchingSystem;

    UPROPERTY()
    class UTranspersonalGameState* GameState;

    // Animation Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementProperties();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateIKProperties();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalProperties();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatProperties();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateEnvironmentalProperties();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float CalculateGroundSlope();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CheckUnevenTerrain();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBodyIK();

    // Animation Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnMovementStateChanged(EAnim_MovementState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnCombatStateChanged(bool bInCombat);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation Events")
    void OnInjuryStateChanged(bool bInjured);

private:
    // Internal state tracking
    EAnim_MovementState PreviousMovementState;
    bool bPreviousCombatState;
    bool bPreviousInjuryState;
    
    // Smoothing values
    float SpeedSmoothRate;
    float DirectionSmoothRate;
    float IKSmoothRate;
};