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
struct TRANSPERSONALGAME_API FAnim_MovementData
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

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    float JumpVelocityThreshold = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    float FallVelocityThreshold = -100.0f;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void CalculateDirection();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterWalkState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterRunState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterJumpState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterFallState() const;

private:
    void UpdateMovementData();
    void UpdateCharacterReferences();
};