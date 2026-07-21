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
    bool bIsSwimming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsSwimming = false;
        MovementState = EAnim_MovementState::Idle;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Settings")
    float JumpVelocityThreshold = 50.0f;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState CalculateMovementState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayWalkAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayRunAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayJumpAnimation() const;
};