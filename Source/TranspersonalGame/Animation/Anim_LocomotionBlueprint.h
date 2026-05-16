#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_LocomotionBlueprint.generated.h"

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
    EAnim_LocomotionState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float JumpHeight;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float FallVelocity;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_LocomotionState::Idle;
        JumpHeight = 0.0f;
        FallVelocity = 0.0f;
    }
};

/**
 * Animation Blueprint for character locomotion system
 * Handles idle, walk, run, jump, and landing animations
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_LocomotionBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_LocomotionBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    class UCharacterMovementComponent* MovementComponent;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpThreshold;

    // Animation sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* CrouchWalkAnimation;

private:
    void UpdateLocomotionData();
    void UpdateAnimationState();
    EAnim_LocomotionState CalculateLocomotionState() const;

public:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAnimationSequence(EAnim_LocomotionState State, UAnimSequence* Animation);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_LocomotionData GetLocomotionData() const { return LocomotionData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayWalkAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayRunAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayJumpAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayCrouchAnimation() const;
};