#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouched = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimYaw = 0.0f;
};

/**
 * Core animation instance for TranspersonalCharacter
 * Handles movement state detection, blend space calculations, and animation transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation Blueprint Event Graph functions
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement data calculation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceValues();

    // State detection helpers
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterMovementState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterJumpState() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldEnterFallState() const;

public:
    // Main movement data structure
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwnerCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // Animation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SpeedSmoothingRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionSmoothingRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float LeanSmoothingRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SprintSpeedThreshold = 600.0f;

    // Jump and fall detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float JumpVelocityThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FallVelocityThreshold = -100.0f;

    // Aim offset values
    UPROPERTY(BlueprintReadOnly, Category = "Aim Offset")
    float AimOffsetPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Aim Offset")
    float AimOffsetYaw = 0.0f;

    // Animation state booleans for state machine
    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bShouldEnterIdle = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bShouldEnterMovement = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bShouldEnterJump = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bShouldEnterFall = false;

private:
    // Smoothed values for blend spaces
    float SmoothedSpeed = 0.0f;
    float SmoothedDirection = 0.0f;
    float SmoothedLeanAngle = 0.0f;
    
    // Previous frame values for delta calculations
    FVector PreviousVelocity = FVector::ZeroVector;
    EAnim_MovementState PreviousMovementState = EAnim_MovementState::Idle;
    
    // Timing for state transitions
    float TimeInCurrentState = 0.0f;
    float MinStateTime = 0.1f; // Minimum time to stay in a state before transitioning
};