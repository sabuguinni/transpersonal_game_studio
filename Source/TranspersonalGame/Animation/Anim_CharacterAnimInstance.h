#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"), 
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion") 
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_MovementState MovementState;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

/**
 * Animation Instance for TranspersonalCharacter
 * Handles locomotion blending and state management for realistic character movement
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    // Movement component reference  
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Locomotion data for animation blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_LocomotionData LocomotionData;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data") 
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bShouldMove;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    bool bIsJumping;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

private:
    // Update locomotion data from character
    void UpdateLocomotionData();
    
    // Determine current movement state
    EAnim_MovementState CalculateMovementState() const;
    
    // Calculate movement direction relative to character facing
    float CalculateDirection() const;
};