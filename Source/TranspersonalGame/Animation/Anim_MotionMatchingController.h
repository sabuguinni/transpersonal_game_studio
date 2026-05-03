#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsOnGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_MotionState CurrentState = EAnim_MotionState::Idle;

    FAnim_MotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Velocity = FVector::ZeroVector;
        bIsOnGround = true;
        bIsMoving = false;
        CurrentState = EAnim_MotionState::Idle;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MotionState DetermineMotionState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToState(EAnim_MotionState NewState);

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMotionState(EAnim_MotionState NewState);

protected:
    // Current motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData CurrentMotionData;

    // Previous motion data for smooth transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Data")
    FAnim_MotionData PreviousMotionData;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Thresholds")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Thresholds")
    float RunSpeedThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Thresholds")
    float IdleSpeedThreshold = 10.0f;

    // State transition delays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Timing")
    float StateTransitionDelay = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Timing")
    float MotionSmoothingFactor = 5.0f;

    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

private:
    // Internal state tracking
    float StateTransitionTimer = 0.0f;
    EAnim_MotionState PendingState = EAnim_MotionState::Idle;
    bool bHasPendingTransition = false;

    // Helper functions
    void CacheComponentReferences();
    void SmoothMotionData(float DeltaTime);
    bool ShouldTransitionToState(EAnim_MotionState NewState);
    void ProcessStateTransition(float DeltaTime);
};