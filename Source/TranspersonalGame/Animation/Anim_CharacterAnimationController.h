#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_CharacterAnimationController.generated.h"

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

USTRUCT(BlueprintType)
struct FAnim_MovementData
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
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsAccelerating = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimationController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* RunAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* JumpStartAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* JumpLoopAnimation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Sequences")
    class UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionDeadZone = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSequences();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterWalkState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterRunState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterJumpState() const;

private:
    void UpdateMovementData();
    void CalculateDirection();
    EAnim_MovementState DetermineMovementState() const;
};