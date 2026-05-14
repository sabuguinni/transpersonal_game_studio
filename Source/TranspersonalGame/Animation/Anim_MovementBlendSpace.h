#pragma once

#include "CoreMinimal.h"
#include "Engine/BlendSpace1D.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "Anim_MovementBlendSpace.generated.h"

/**
 * Movement Blend Space Controller for Transpersonal Character
 * Manages 1D blend space for idle/walk/run transitions based on velocity
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Animation))
class TRANSPERSONALGAME_API UAnim_MovementBlendSpace : public UObject
{
    GENERATED_BODY()

public:
    UAnim_MovementBlendSpace();

    // Movement speed thresholds for blend space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float IdleThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SprintThreshold = 600.0f;

    // Current movement state
    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float CurrentSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    float BlendSpaceInput = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement State")
    bool bIsSprinting = false;

    // Animation references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    class UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    class UAnimSequence* WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    class UAnimSequence* RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    class UAnimSequence* SprintAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    class UBlendSpace1D* MovementBlendSpace;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementState(float Velocity, bool bIsInAir);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float CalculateBlendSpaceInput(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementThresholds(float NewWalkThreshold, float NewRunThreshold, float NewSprintThreshold);

    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsInMovementState(const FString& StateName) const;

protected:
    // Internal state tracking
    float PreviousSpeed = 0.0f;
    float SpeedChangeRate = 0.0f;
    
    // Smoothing for blend space input
    UPROPERTY(EditAnywhere, Category = "Smoothing")
    float BlendSpaceSmoothingSpeed = 5.0f;

    UFUNCTION()
    void SmoothBlendSpaceTransition(float DeltaTime);
};

/**
 * Enum for movement animation states
 */
UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

/**
 * Struct for movement animation data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    FAnim_MovementData()
    {
        CurrentState = EAnim_MovementState::Idle;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        AirTime = 0.0f;
        Velocity = FVector::ZeroVector;
    }
};

#include "Anim_MovementBlendSpace.generated.h"