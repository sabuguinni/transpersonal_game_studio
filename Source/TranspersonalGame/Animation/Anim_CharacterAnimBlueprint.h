#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimBlueprint.generated.h"

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
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

/**
 * Animation Blueprint for TranspersonalCharacter
 * Handles state machine for idle, walk, run, jump animations
 * Implements Motion Matching principles for fluid movement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Movement data for animation state machine
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float JumpVelocityThreshold;

    // Animation blending
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float IdleToWalkBlendTime;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float WalkToRunBlendTime;

private:
    void UpdateMovementData();
    void UpdateMovementState();
    EAnim_MovementState CalculateMovementState() const;
};