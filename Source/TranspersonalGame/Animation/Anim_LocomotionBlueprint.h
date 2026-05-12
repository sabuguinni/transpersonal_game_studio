#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Anim_LocomotionBlueprint.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
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
    EAnim_LocomotionState CurrentState = EAnim_LocomotionState::Idle;
};

/**
 * Animation Blueprint for prehistoric character locomotion
 * Handles state transitions and movement blending for survival gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_LocomotionBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_LocomotionBlueprint();

    // Animation Blueprint interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Movement data for animation logic
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Assets")
    class UAnimMontage* LandingMontage;

    // Animation parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float WalkSpeedThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings")
    float MovementInterpolationSpeed = 10.0f;

private:
    // Internal state tracking
    EAnim_LocomotionState PreviousState;
    float StateChangeTime;
    bool bWasInAir;

    // Helper functions
    void UpdateMovementData();
    void UpdateLocomotionState();
    EAnim_LocomotionState CalculateDesiredState() const;
    void HandleStateTransition(EAnim_LocomotionState NewState);

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandingAnimation();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const { return MovementData.Speed > 10.0f; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsRunning() const { return MovementData.Speed > RunSpeedThreshold; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_LocomotionState GetCurrentState() const { return MovementData.CurrentState; }
};