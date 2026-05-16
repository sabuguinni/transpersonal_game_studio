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

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
    }
};

/**
 * Animation Blueprint class for TranspersonalCharacter
 * Handles state machine transitions and movement-based animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimBlueprint();

protected:
    // Animation update functions
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwnerCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Thresholds")
    float IdleSpeedThreshold;

public:
    // Animation state functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldPlayIdleAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldPlayWalkAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldPlayRunAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldPlayJumpAnimation() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementSpeed() const;

    // Animation montage functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandMontage();

private:
    // Internal state tracking
    EAnim_MovementState PreviousMovementState;
    float StateChangeTimer;
    bool bWasInAir;
    
    // Animation assets
    UPROPERTY(EditAnywhere, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, Category = "Animation Assets", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* LandMontage;
};