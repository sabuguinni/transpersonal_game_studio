#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingController.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MotionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
    EAnim_LocomotionState CurrentState;

    FAnim_MotionData()
    {
        Velocity = FVector::ZeroVector;
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        CurrentState = EAnim_LocomotionState::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* IdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* WalkMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* RunMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UBlendSpace* LocomotionBlendSpace;

    FAnim_MontageSet()
    {
        IdleMontage = nullptr;
        WalkMontage = nullptr;
        RunMontage = nullptr;
        JumpMontage = nullptr;
        LocomotionBlendSpace = nullptr;
    }
};

/**
 * Advanced Motion Matching Controller for realistic character animation
 * Handles smooth transitions between animation states based on movement data
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_MotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_MotionMatchingController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core motion data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData CurrentMotionData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Matching")
    FAnim_MotionData PreviousMotionData;

    // Animation assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    FAnim_MontageSet MontageSet;

    // Motion matching parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TransitionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    float RunSpeedThreshold;

    // Character references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    UAnimInstance* AnimInstance;

public:
    // Motion matching functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionData();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    EAnim_LocomotionState DetermineLocomotionState();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void TransitionToState(EAnim_LocomotionState NewState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void PlayMontageForState(EAnim_LocomotionState State);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    float CalculateDirectionAngle();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    bool ShouldTransitionToState(EAnim_LocomotionState TargetState);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    FAnim_MotionData GetCurrentMotionData() const { return CurrentMotionData; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    EAnim_LocomotionState GetCurrentState() const { return CurrentMotionData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetCurrentSpeed() const { return CurrentMotionData.Speed; }

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    bool IsInTransition() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void SetMontageSet(const FAnim_MontageSet& NewMontageSet);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void InitializeWithCharacter(ACharacter* Character);

private:
    // Internal state tracking
    float StateTransitionTimer;
    bool bIsTransitioning;
    EAnim_LocomotionState PendingState;

    // Helper functions
    void InitializeReferences();
    void ValidateAnimationAssets();
    bool IsValidTransition(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState);
    float GetTransitionDuration(EAnim_LocomotionState FromState, EAnim_LocomotionState ToState);
};