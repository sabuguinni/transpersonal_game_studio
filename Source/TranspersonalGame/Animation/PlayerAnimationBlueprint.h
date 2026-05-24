#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerAnimationBlueprint.generated.h"

/**
 * Animation Blueprint for the player character
 * Handles locomotion states, survival animations, and environmental reactions
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UPlayerAnimationBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimationBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Movement variables
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    // Survival states
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HungerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float ThirstLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    // Environmental reactions
    UPROPERTY(BlueprintReadOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    float GroundSlope;

    UPROPERTY(BlueprintReadOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    bool bIsOnRoughTerrain;

    UPROPERTY(BlueprintReadOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    bool bIsInWater;

    // Animation state
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    bool bShouldPlayFearAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    bool bShouldPlayPainAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float AnimationBlendWeight;

private:
    // Reference to the character
    UPROPERTY()
    class ACharacter* Character;

    // Reference to movement component
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Helper functions
    void UpdateLocomotionVariables();
    void UpdateSurvivalStates();
    void UpdateEnvironmentalReactions();
    void UpdateAnimationStates();

    // Calculate ground slope
    float CalculateGroundSlope() const;
    
    // Check terrain type
    bool IsOnRoughTerrain() const;
};