#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPCPersonality : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Aggressive      UMETA(DisplayName = "Aggressive"), 
    Curious         UMETA(DisplayName = "Curious"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary")
};

UENUM(BlueprintType)
enum class ENPCState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Eating          UMETA(DisplayName = "Eating"),
    Hunting         UMETA(DisplayName = "Hunting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> InterestingLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> ActorThreatLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerIsKnown;

    FNPCMemory()
    {
        LastPlayerInteractionTime = 0.0f;
        bPlayerIsKnown = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duration in hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ActivityDescription;

    FNPCRoutine()
    {
        StartTime = 8.0f;
        Duration = 2.0f;
        Activity = ENPCState::Working;
        TargetLocation = FVector::ZeroVector;
        ActivityDescription = TEXT("Generic Activity");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core NPC Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    int32 NPCAge;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPCMemory NPCMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime;

    // Daily Routine System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPCRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bFollowDailyRoutine;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> KnownNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> RelationshipValues; // -1.0 (enemy) to 1.0 (friend)

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngle;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetNPCState(ENPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float ThreatLevel = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsActorKnown(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPCRoutine GetCurrentRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void AddRoutine(const FNPCRoutine& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(AActor* OtherActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipValue(AActor* OtherActor) const;

    // AI Integration
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

protected:
    // Internal Functions
    void UpdateDailyRoutine();
    void ProcessMemory(float DeltaTime);
    void HandlePerception();
    float GetCurrentTimeOfDay() const;
    
    // Timers
    float MemoryCleanupTimer;
    float RoutineUpdateTimer;
    
    // References
    UPROPERTY()
    class AAIController* NPCAIController;
    
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
};