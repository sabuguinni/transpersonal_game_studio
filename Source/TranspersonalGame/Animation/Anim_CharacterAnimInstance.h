#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
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
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;
};

/**
 * Professional Animation Instance for TranspersonalCharacter
 * Handles all character animation states and transitions
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Movement Data
    UPROPERTY(BlueprintReadOnly, Category = "Animation Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character References
    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* CharacterMovement;

    // Animation Properties
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float JumpingThreshold = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float LandingThreshold = -200.0f;

    // State Transition Smoothing
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float StateTransitionSpeed = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float SpeedSmoothingRate = 10.0f;

private:
    // Internal state tracking
    EAnim_MovementState PreviousMovementState;
    float SmoothedSpeed;
    float StateTransitionAlpha;

    // Update functions
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateGroundDistance();
    EAnim_MovementState CalculateMovementState() const;
    float CalculateDirection() const;

public:
    // Blueprint accessible getters
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE float GetSpeed() const { return MovementData.Speed; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE float GetDirection() const { return MovementData.Direction; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE bool IsInAir() const { return MovementData.bIsInAir; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE bool IsCrouching() const { return MovementData.bIsCrouching; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE bool IsAccelerating() const { return MovementData.bIsAccelerating; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE EAnim_MovementState GetMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE float GetGroundDistance() const { return MovementData.GroundDistance; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    FORCEINLINE float GetSmoothedSpeed() const { return SmoothedSpeed; }
};