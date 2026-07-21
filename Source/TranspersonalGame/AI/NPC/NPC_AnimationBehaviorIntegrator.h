#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "SharedTypes.h"
#include "NPC_AnimationBehaviorIntegrator.generated.h"

// Forward declarations
class UNPC_DinosaurBehaviorIntegrator;
class UAnim_MotionMatchingComponent;
class UAnim_IKFootPlacementComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_AnimationBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    ENPC_DinosaurEmotionalState EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Behavior")
    FVector TargetLocation;

    FNPC_AnimationBehaviorState()
    {
        CurrentBehaviorState = ENPC_DinosaurBehaviorState::Idle;
        EmotionalState = ENPC_DinosaurEmotionalState::Calm;
        MovementSpeed = 0.0f;
        AggressionLevel = 0.0f;
        bIsInCombat = false;
        bIsHunting = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_AnimationTransitionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Transition")
    ENPC_DinosaurBehaviorState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Transition")
    ENPC_DinosaurBehaviorState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Transition")
    float TransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Transition")
    bool bRequiresBlending;

    FNPC_AnimationTransitionRule()
    {
        FromState = ENPC_DinosaurBehaviorState::Idle;
        ToState = ENPC_DinosaurBehaviorState::Idle;
        TransitionDuration = 0.5f;
        bRequiresBlending = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_AnimationBehaviorIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AnimationBehaviorIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Animation-Behavior Integration
    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateEmotionalState(ENPC_DinosaurEmotionalState NewEmotionalState);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void UpdateMovementParameters(float Speed, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void TriggerBehaviorAnimation(ENPC_DinosaurBehaviorState BehaviorState);

    UFUNCTION(BlueprintCallable, Category = "Animation Behavior")
    void BlendToNewBehavior(ENPC_DinosaurBehaviorState NewBehavior, float BlendTime = 0.5f);

    // Motion Matching Integration
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ConfigureMotionMatchingForBehavior(ENPC_DinosaurBehaviorState BehaviorState);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateMotionMatchingDatabase();

    // IK Integration
    UFUNCTION(BlueprintCallable, Category = "IK Integration")
    void EnableIKForBehavior(ENPC_DinosaurBehaviorState BehaviorState);

    UFUNCTION(BlueprintCallable, Category = "IK Integration")
    void UpdateIKTargets(const TArray<FVector>& FootTargets);

    // Behavior-Specific Animations
    UFUNCTION(BlueprintCallable, Category = "Behavior Animations")
    void PlayHuntingAnimation();

    UFUNCTION(BlueprintCallable, Category = "Behavior Animations")
    void PlayTerritorialAnimation();

    UFUNCTION(BlueprintCallable, Category = "Behavior Animations")
    void PlayPackCoordinationAnimation();

    UFUNCTION(BlueprintCallable, Category = "Behavior Animations")
    void PlayEmotionalResponseAnimation(ENPC_DinosaurEmotionalState EmotionalState);

protected:
    // Animation State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    FNPC_AnimationBehaviorState CurrentAnimationState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation State")
    TArray<FNPC_AnimationTransitionRule> TransitionRules;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UNPC_DinosaurBehaviorIntegrator* BehaviorIntegrator;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UAnim_MotionMatchingComponent* MotionMatchingComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UAnim_IKFootPlacementComponent* IKComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UAnimInstance* AnimInstance;

    // Animation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DefaultBlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    bool bEnableIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationUpdateRate;

private:
    // Internal Methods
    void InitializeComponents();
    void UpdateAnimationParameters();
    bool CanTransitionToBehavior(ENPC_DinosaurBehaviorState NewBehavior);
    FNPC_AnimationTransitionRule* FindTransitionRule(ENPC_DinosaurBehaviorState From, ENPC_DinosaurBehaviorState To);
    void ApplyBehaviorToAnimation();

    // Animation Timers
    float LastAnimationUpdate;
    float AnimationBlendTimer;
    bool bIsTransitioning;
};