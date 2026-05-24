#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

/**
 * Studio Director Coordinator - Central coordination system for all 19 agents
 * Manages task distribution, progress tracking, and agent communication
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchTaskToAgent(int32 AgentNumber, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportAgentProgress(int32 AgentNumber, float ProgressPercent, const FString& StatusMessage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinimumViablePrototype();

    // Production milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckMilestone1WalkAround();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SpawnDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePlayerMovement();

protected:
    // Agent status tracking
    UPROPERTY(BlueprintReadWrite, Category = "Agent Status")
    TArray<FDir_AgentStatus> AgentStatusList;

    // Production metrics
    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 TotalActorsSpawned;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    int32 DinosaurActorsCount;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bPlayerMovementWorking;

    UPROPERTY(BlueprintReadWrite, Category = "Production")
    bool bMilestone1Complete;

    // Task distribution
    UPROPERTY(BlueprintReadWrite, Category = "Tasks")
    TArray<FDir_AgentTask> PendingTasks;

    UPROPERTY(BlueprintReadWrite, Category = "Tasks")
    TArray<FDir_AgentTask> CompletedTasks;

private:
    // Internal coordination logic
    void UpdateAgentChain();
    void ProcessTaskQueue();
    void ValidateProductionState();
    bool CheckAgentDependencies(int32 AgentNumber);
    void LogProductionMetrics();
};