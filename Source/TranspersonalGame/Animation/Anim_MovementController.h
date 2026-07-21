#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Character/TranspersonalCharacter.h"
#include "Anim_MovementController.generated.h"

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
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float JumpHeight = 0.0f;

    FAnim_MovementData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsAccelerating = false;
        MovementState = EAnim_MovementState::Idle;
        Velocity = FVector::ZeroVector;
        GroundSpeed = 0.0f;
        JumpHeight = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MovementController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MovementController();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class ATranspersonalCharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    class UCharacterMovementComponent* CharacterMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementData MovementData;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IdleThreshold = 10.0f;

    // Animation blend parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float IdleToWalkBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float WalkToRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Blending")
    float DirectionalBlend = 0.0f;

private:
    void UpdateMovementData(float DeltaTime);
    void UpdateMovementState();
    void UpdateBlendParameters(float DeltaTime);
    EAnim_MovementState DetermineMovementState() const;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsMoving() const { return MovementData.Speed > IdleThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsRunning() const { return MovementData.Speed > RunSpeedThreshold; }

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ForceUpdateAnimation();
};