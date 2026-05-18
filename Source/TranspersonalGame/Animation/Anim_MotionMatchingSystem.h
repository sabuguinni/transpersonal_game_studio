#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingSystem.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Sprinting       UMETA(DisplayName = "Sprinting"),
    Jumping         UMETA(DisplayName = "Jumping"),
    Falling         UMETA(DisplayName = "Falling"),
    Landing         UMETA(DisplayName = "Landing"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Climbing        UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    float DistanceToGround = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    EAnim_MovementState CurrentState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Data")
    float StateTransitionTime = 0.0f;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion data tracking
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData PreviousMotionData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimMontage* LandingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    class UAnimSequence* IdleAnimation;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching Settings")
    float StateTransitionSmoothness = 0.2f;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class ACharacter* OwnerCharacter;

public:
    // Public interface
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_MovementState GetCurrentMovementState() const { return CurrentMotionData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetLocomotionBlendSpace(class UBlendSpace* NewBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayLandingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool IsTransitioningStates() const;

private:
    void UpdateMotionData(float DeltaTime);
    void UpdateMovementState();
    void PerformGroundTrace();
    EAnim_MovementState DetermineMovementState() const;
    void HandleStateTransition(EAnim_MovementState NewState);
    float CalculateDirection() const;
};