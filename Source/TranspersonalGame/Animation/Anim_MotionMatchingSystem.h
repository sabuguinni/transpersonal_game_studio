#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Prone       UMETA(DisplayName = "Prone")
};

UENUM(BlueprintType)
enum class EAnim_MovementDirection : uint8
{
    Forward     UMETA(DisplayName = "Forward"),
    Backward    UMETA(DisplayName = "Backward"),
    Left        UMETA(DisplayName = "Left"),
    Right       UMETA(DisplayName = "Right"),
    ForwardLeft UMETA(DisplayName = "Forward Left"),
    ForwardRight UMETA(DisplayName = "Forward Right"),
    BackwardLeft UMETA(DisplayName = "Backward Left"),
    BackwardRight UMETA(DisplayName = "Backward Right")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Motion")
    EAnim_MovementDirection MovementDirection = EAnim_MovementDirection::Forward;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsAccelerating = false;
        bIsInAir = false;
        bIsCrouching = false;
        LocomotionState = EAnim_LocomotionState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetLocomotionState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_LocomotionState GetCurrentLocomotionState() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsMoving() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData MotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character Reference")
    UCharacterMovementComponent* CharacterMovement;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    UAnimMontage* LandMontage;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirectionSmoothingSpeed = 10.0f;

private:
    void CalculateLocomotionState();
    void CalculateMovementDirection();
    float CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const;
};