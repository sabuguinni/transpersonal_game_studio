#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterLocomotion.generated.h"

/**
 * Core locomotion animation instance for prehistoric survival characters
 * Handles basic movement states: idle, walk, run, jump, crouch
 * Optimized for performance with minimal state complexity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterLocomotion : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterLocomotion();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement properties
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

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float WalkThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float RunThreshold = 300.0f;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

private:
    void UpdateMovementProperties();
    void UpdateAirState();
    void UpdateAcceleration();
    
    FVector LastVelocity;
    float AccelerationThreshold = 10.0f;
};