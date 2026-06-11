#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "NPCBehaviorController.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPCBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardData;

    // NPC Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float RunSpeed;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate;

public:
    // Behavior Control Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartFleeBehavior(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartIdleBehavior();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FNPC_MemoryEntry& NewMemory);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMemoryOfActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ClearMemories();

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void InitializeAIPerception();
    void UpdateMemorySystem(float DeltaTime);
    void ProcessShortTermMemory();
    void ConsolidateToLongTermMemory();

private:
    // Internal State
    FVector LastKnownPlayerLocation;
    float TimeSinceLastPlayerSighting;
    bool bPlayerInSight;
    
    // Patrol System
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex;
    FVector HomeLocation;
    
    void GeneratePatrolPoints();
    FVector GetNextPatrolPoint();
};