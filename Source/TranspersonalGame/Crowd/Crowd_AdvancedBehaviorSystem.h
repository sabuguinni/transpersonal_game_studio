#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Crowd_AdvancedBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle,
    Wandering,
    Following,
    Fleeing,
    Gathering,
    Working,
    Socializing,
    Hunting,
    Feeding,
    Resting
};

UENUM(BlueprintType)
enum class ECrowd_EmotionalState : uint8
{
    Calm,
    Curious,
    Fearful,
    Aggressive,
    Excited,
    Tired,
    Hungry,
    Social
};

USTRUCT(BlueprintType)
struct FCrowd_BehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float EnergyLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_BehaviorRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    ECrowd_BehaviorState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    ECrowd_BehaviorState TargetState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float Probability = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float MinDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float MaxDistance = 500.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_AdvancedBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_AdvancedBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior System")
    TArray<FCrowd_BehaviorRule> BehaviorRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior System")
    FCrowd_BehaviorProfile DefaultProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior System")
    float BehaviorUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior System")
    float EmotionalDecayRate = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ECrowd_BehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ECrowd_EmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FCrowd_BehaviorProfile CurrentProfile;

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void SetBehaviorState(ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void SetEmotionalState(ECrowd_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void UpdateBehaviorProfile(const FCrowd_BehaviorProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void AddBehaviorRule(const FCrowd_BehaviorRule& NewRule);

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    ECrowd_BehaviorState EvaluateNextBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void ProcessEnvironmentalStimuli(const TArray<AActor*>& NearbyActors);

    UFUNCTION(BlueprintCallable, Category = "Behavior System")
    void ApplyEmotionalInfluence(ECrowd_EmotionalState Influence, float Intensity);

private:
    float LastBehaviorUpdate;
    float EmotionalIntensity;
    TArray<AActor*> CachedNearbyActors;

    void UpdateEmotionalState(float DeltaTime);
    void ProcessBehaviorTransitions();
    bool EvaluateBehaviorRule(const FCrowd_BehaviorRule& Rule, const TArray<AActor*>& Context);
};