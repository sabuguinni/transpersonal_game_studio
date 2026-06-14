#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

USTRUCT(BlueprintType)
struct FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Aggression = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Curiosity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Fatigue = 0.0f;

    FNPC_EmotionalState()
    {
        Fear = 0.0f;
        Aggression = 0.0f;
        Curiosity = 0.0f;
        Hunger = 0.0f;
        Fatigue = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString EventType;

    FNPC_Memory()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        Importance = 0.0f;
        EventType = TEXT("None");
    }
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Fleeing,
    Attacking,
    Feeding,
    Resting,
    Socializing
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Behavior State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_EmotionalState EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FNPC_Memory> Memories;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector HomeLocation;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddMemory(FVector Location, FString EventType, float Importance);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateEmotionalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* DetectNearbyThreats();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ExecuteBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetPatrolTarget();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldAttack();

private:
    float StateTimer;
    FVector CurrentTarget;
    AActor* CurrentThreat;
    
    void ProcessIdleBehavior(float DeltaTime);
    void ProcessPatrolBehavior(float DeltaTime);
    void ProcessInvestigatingBehavior(float DeltaTime);
    void ProcessFleeingBehavior(float DeltaTime);
    void ProcessAttackingBehavior(float DeltaTime);
    void ProcessFeedingBehavior(float DeltaTime);
    void ProcessRestingBehavior(float DeltaTime);
    void ProcessSocializingBehavior(float DeltaTime);
};