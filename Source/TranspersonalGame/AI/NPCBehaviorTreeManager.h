#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPCBehaviorTreeManager.generated.h"

class APawn;
class AAIController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Intensity;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        MemoryType = ENPC_MemoryType::Neutral;
        Intensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_RelationType RelationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    FNPC_SocialRelation()
    {
        TargetPawn = nullptr;
        RelationshipValue = 0.0f;
        RelationType = ENPC_RelationType::Neutral;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorTree(UBehaviorTree* NewBehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree, bool bForceRestart = true);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void PauseBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ResumeBehaviorTree();

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(FVector Location, ENPC_MemoryType MemoryType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesInRadius(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_MemoryEntry GetStrongestMemory(ENPC_MemoryType MemoryType);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ClearOldMemories(float MaxAge = 300.0f);

    // Social System
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void UpdateRelationship(APawn* TargetPawn, float DeltaValue, ENPC_RelationType NewRelationType = ENPC_RelationType::Neutral);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    FNPC_SocialRelation GetRelationship(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    TArray<FNPC_SocialRelation> GetAllRelationships();

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ProcessSocialInteraction(APawn* TargetPawn, ENPC_InteractionType InteractionType);

    // Daily Routine System
    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void SetDailyRoutine(const TArray<FNPC_RoutineTask>& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    FNPC_RoutineTask GetCurrentRoutineTask();

    UFUNCTION(BlueprintCallable, Category = "NPC Routine")
    void AdvanceToNextRoutineTask();

    // Blackboard Management
    UFUNCTION(BlueprintCallable, Category = "NPC Blackboard")
    void SetBlackboardValue(FName KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Blackboard")
    void SetBlackboardVector(FName KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Blackboard")
    void SetBlackboardObject(FName KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Blackboard")
    float GetBlackboardValue(FName KeyName);

    // State Management
    UFUNCTION(BlueprintCallable, Category = "NPC State")
    void SetNPCState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC State")
    ENPC_BehaviorState GetNPCState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC State")
    bool IsInCombat() const { return CurrentBehaviorState == ENPC_BehaviorState::Combat; }

    UFUNCTION(BlueprintCallable, Category = "NPC State")
    bool IsPatrolling() const { return CurrentBehaviorState == ENPC_BehaviorState::Patrol; }

protected:
    // Core References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* CurrentBehaviorTree;

    // Memory System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Social System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialUpdateInterval;

    // Daily Routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPC_RoutineTask> DailyRoutine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine")
    int32 CurrentRoutineIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Routine")
    float RoutineStartTime;

    // State Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ENPC_BehaviorState PreviousBehaviorState;

    // Timers
    FTimerHandle MemoryCleanupTimer;
    FTimerHandle SocialUpdateTimer;
    FTimerHandle RoutineUpdateTimer;

private:
    // Internal Methods
    void UpdateMemorySystem(float DeltaTime);
    void UpdateSocialSystem(float DeltaTime);
    void UpdateRoutineSystem(float DeltaTime);
    void CleanupExpiredMemories();
    void DecayMemoryIntensity(float DeltaTime);
    AAIController* GetOwnerAIController();
};