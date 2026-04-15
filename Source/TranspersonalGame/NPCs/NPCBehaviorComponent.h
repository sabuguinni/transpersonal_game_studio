#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

// NPC-specific enums
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0            UMETA(DisplayName = "Idle"),
    Working = 1         UMETA(DisplayName = "Working"),
    Socializing = 2     UMETA(DisplayName = "Socializing"),
    Eating = 3          UMETA(DisplayName = "Eating"),
    Sleeping = 4        UMETA(DisplayName = "Sleeping"),
    Patrolling = 5      UMETA(DisplayName = "Patrolling"),
    Fleeing = 6         UMETA(DisplayName = "Fleeing"),
    Fighting = 7        UMETA(DisplayName = "Fighting"),
    Gathering = 8       UMETA(DisplayName = "Gathering"),
    Hunting = 9         UMETA(DisplayName = "Hunting")
};

UENUM(BlueprintType)
enum class ENPC_Personality : uint8
{
    Aggressive = 0      UMETA(DisplayName = "Aggressive"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Friendly = 2        UMETA(DisplayName = "Friendly"),
    Curious = 3         UMETA(DisplayName = "Curious"),
    Lazy = 4            UMETA(DisplayName = "Lazy"),
    Hardworking = 5     UMETA(DisplayName = "Hardworking"),
    Leader = 6          UMETA(DisplayName = "Leader"),
    Follower = 7        UMETA(DisplayName = "Follower")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_Personality Personality = ENPC_Personality::Friendly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Sociability = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WorkEthic = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CurrentMood = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Energy = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Hunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Fear = 0.0f;

    FNPC_BehaviorData()
    {
        CurrentState = ENPC_BehaviorState::Idle;
        Personality = ENPC_Personality::Friendly;
        Aggression = 0.3f;
        Sociability = 0.7f;
        Curiosity = 0.5f;
        WorkEthic = 0.6f;
        CurrentMood = 0.5f;
        Energy = 1.0f;
        Hunger = 0.0f;
        Fear = 0.0f;
    }
};

/**
 * Core NPC Behavior Component
 * Manages the fundamental behavior state and personality of NPCs
 */
UCLASS(ClassGroup=(NPCs), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_BehaviorData BehaviorData;

    // NPC identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 TribeID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age = 25;

    // Behavior timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float StateChangeInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Timing")
    float LastStateChangeTime = 0.0f;

    // Decision making
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    float DecisionThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision Making")
    TArray<ENPC_BehaviorState> AvailableStates;

    // Public interface
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(float MoodChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldChangeState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState ChooseNewState();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToThreat(AActor* Threat);

private:
    void InitializeDefaultStates();
    void UpdateBehaviorLogic(float DeltaTime);
    float CalculateStateDesirability(ENPC_BehaviorState State) const;
};