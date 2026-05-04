#pragma once

#include "CoreMinimal.h"
#include "Engine/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "SharedTypes.h"
#include "ProductionCoordinator.generated.h"

/**
 * Production Coordinator Actor
 * Manages and tracks Milestone 1 progress across all production agents
 * Provides real-time status updates and coordinates task dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* StatusDisplay;

    // Milestone 1 tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bCharacterMovementComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bTerrainExpanded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bDinosaurSpawnsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bLightingFixed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone 1")
    bool bHUDImplemented;

    // Agent task assignments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Coordination")
    TArray<FString> BlockedTasks;

public:
    // Task management functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& TaskName, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddPendingTask(const FString& TaskName, int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsTaskComplete(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetCurrentPriorityTask();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogProductionStatus();

    // Agent coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FString> GetTasksForAgent(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void ReportAgentProgress(int32 AgentID, const FString& TaskName, float Progress);

private:
    void UpdateStatusDisplay();
    void CheckMilestoneCompletion();
    
    // Internal tracking
    float LastUpdateTime;
    int32 TotalMilestoneTasks;
    int32 CompletedMilestoneTasks;
};