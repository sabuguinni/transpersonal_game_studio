#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame.h"
#include "Anim_PlayerLocomotionBlueprint.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
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
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    EAnim_LocomotionState CurrentState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Locomotion")
    EAnim_MovementDirection MovementDirection = EAnim_MovementDirection::Forward;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        LeanAngle = 0.0f;
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        CurrentState = EAnim_LocomotionState::Idle;
        MovementDirection = EAnim_MovementDirection::Forward;
    }
};

/**
 * Animation Blueprint class for player locomotion
 * Handles basic movement states: idle, walk, run, jump, crouch
 * Provides smooth blending between animation states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PlayerLocomotionBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PlayerLocomotionBlueprint();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Core locomotion data
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionData LocomotionData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwningCharacter;

    // Movement component reference
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    class UCharacterMovementComponent* MovementComponent;

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionDeadZone = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float LeanInterpSpeed = 5.0f;

    // State transition smoothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float StateTransitionSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float VelocityInterpSpeed = 6.0f;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLeanAngle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterIdleState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterWalkState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterRunState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterJumpState() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldEnterCrouchState() const;

    // Getters for Blueprint
    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetSpeed() const { return LocomotionData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetDirection() const { return LocomotionData.Direction; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInAir() const { return LocomotionData.bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsCrouching() const { return LocomotionData.bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLeanAngle() const { return LocomotionData.LeanAngle; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_LocomotionState GetCurrentState() const { return LocomotionData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementDirection GetMovementDirection() const { return LocomotionData.MovementDirection; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FVector GetVelocity() const { return LocomotionData.Velocity; }

private:
    // Internal state tracking
    EAnim_LocomotionState PreviousState;
    float StateTransitionTime;
    FVector PreviousVelocity;
    float CurrentLeanTarget;

    // Helper functions
    void InitializeReferences();
    void UpdateMovementData(float DeltaTime);
    void SmoothStateTransitions(float DeltaTime);
    float CalculateDirectionAngle() const;
    EAnim_MovementDirection DetermineMovementDirection(float DirectionAngle) const;
    bool IsMovingForward() const;
    bool IsMovingBackward() const;
    bool IsMovingSideways() const;
};