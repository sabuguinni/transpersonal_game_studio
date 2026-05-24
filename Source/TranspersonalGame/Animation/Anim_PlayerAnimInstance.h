#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching")
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
    float JumpHeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        JumpHeight = 0.0f;
        LocomotionState = EAnim_LocomotionState::Idle;
    }
};

/**
 * Animation Instance for TranspersonalCharacter
 * Handles locomotion state machine and movement blending
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_PlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PlayerAnimInstance();

protected:
    // Animation Blueprint Interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwningCharacter;

    // Movement Component Reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Properties", meta = (AllowPrivateAccess = "true"))
    float WalkThreshold = 50.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Properties", meta = (AllowPrivateAccess = "true"))
    float RunThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Properties", meta = (AllowPrivateAccess = "true"))
    float JumpThreshold = 100.0f;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    bool bWasInAirLastFrame = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    float TimeInCurrentState = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState PreviousState = EAnim_LocomotionState::Idle;

private:
    // Update Functions
    void UpdateMovementData(float DeltaTime);
    void UpdateLocomotionState();
    void UpdateCharacterReferences();

    // State Transition Logic
    bool ShouldTransitionToIdle() const;
    bool ShouldTransitionToWalking() const;
    bool ShouldTransitionToRunning() const;
    bool ShouldTransitionToJumping() const;
    bool ShouldTransitionToFalling() const;
    bool ShouldTransitionToLanding() const;

public:
    // Blueprint Callable Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_LocomotionState GetCurrentLocomotionState() const { return MovementData.LocomotionState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsInAir() const { return MovementData.bIsInAir; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsCrouching() const { return MovementData.bIsCrouching; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ForceLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ResetAnimationState();
};