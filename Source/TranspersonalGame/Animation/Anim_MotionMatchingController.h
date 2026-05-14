#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TranspersonalGame.h"
#include "Anim_MotionMatchingController.generated.h"

UENUM(BlueprintType)
enum class EAnim_MotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Climbing    UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MotionState CurrentState;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_MotionState::Idle;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState DetermineMotionState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateMovementDirection();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetAnimationState(EAnim_MotionState NewState);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EAnim_MotionState GetCurrentState() const { return CurrentMotionData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetMovementSpeed() const { return CurrentMotionData.Speed; }

protected:
    // Current motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    // Previous motion data for smoothing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Character reference
    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Movement component reference
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Smoothing parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
    float SpeedSmoothingRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing")
    float DirectionSmoothingRate = 15.0f;

    // State transition thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float WalkSpeedThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float JumpVelocityThreshold = 100.0f;

    // State change tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float StateChangeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_MotionState PreviousState;

private:
    // Internal helper functions
    void SmoothMotionData(float DeltaTime);
    bool HasStateChanged();
    void OnStateChanged(EAnim_MotionState OldState, EAnim_MotionState NewState);
};