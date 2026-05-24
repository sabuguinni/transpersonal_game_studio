#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Eating UMETA(DisplayName = "Eating"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating"),
    Following UMETA(DisplayName = "Following"),
    Trading UMETA(DisplayName = "Trading")
};

UENUM(BlueprintType)
enum class ENPC_Personality : uint8
{
    Friendly UMETA(DisplayName = "Friendly"),
    Cautious UMETA(DisplayName = "Cautious"),
    Aggressive UMETA(DisplayName = "Aggressive"),
    Curious UMETA(DisplayName = "Curious"),
    Wise UMETA(DisplayName = "Wise"),
    Fearful UMETA(DisplayName = "Fearful"),
    Brave UMETA(DisplayName = "Brave"),
    Lazy UMETA(DisplayName = "Lazy"),
    Hardworking UMETA(DisplayName = "Hardworking"),
    Mysterious UMETA(DisplayName = "Mysterious")
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Relationship;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LastInteraction;

    FNPC_Memory()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Relationship = 0.0f;
        LastSeenTime = 0.0f;
        LastInteraction = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FString ActivityDescription;

    FNPC_DailySchedule()
    {
        StartHour = 8.0f;
        EndHour = 12.0f;
        Activity = ENPC_BehaviorState::Working;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("Work");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_Personality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AwarenessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float MovementSpeed;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Daily Schedule
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FString> KnownDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanTrade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FString> TradeItems;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(AActor* Actor, float Relationship, const FString& Interaction);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_Memory* GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationshipWithActor(AActor* Actor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    TArray<AActor*> GetNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_DailySchedule GetCurrentScheduleActivity();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FString GetRandomDialogue();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitiateTrade(AActor* Trader);

private:
    // Internal state tracking
    float StateTimer;
    AActor* CurrentTarget;
    FVector HomeLocation;
    bool bIsPlayerNearby;
    float LastPlayerInteractionTime;

    // Timer handles
    FTimerHandle ScheduleTimerHandle;
    FTimerHandle MemoryDecayTimerHandle;

    // Internal functions
    void UpdateCurrentActivity();
    void DecayMemories();
    void ProcessNearbyActors();
    void HandlePlayerInteraction();
    void MoveToTarget(const FVector& TargetLocation);
    float GetCurrentGameHour();
};