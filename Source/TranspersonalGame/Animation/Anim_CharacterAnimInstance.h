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

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimPitch;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        MovementState = EAnim_MovementState::Idle;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
        AimYaw = 0.0f;
        AimPitch = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Movement data
    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Animation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionDeadZone;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bWasInAirLastFrame;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeInCurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_MovementState PreviousMovementState;

public:
    // Getters for Blueprint access
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const { return MovementData.Speed > IdleThreshold; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsRunning() const { return MovementData.Speed > RunSpeedThreshold; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsWalking() const { return MovementData.Speed > WalkSpeedThreshold && MovementData.Speed <= RunSpeedThreshold; }

private:
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void CalculateDirection();
    void UpdateAimOffsets();
    EAnim_MovementState DetermineMovementState() const;
};