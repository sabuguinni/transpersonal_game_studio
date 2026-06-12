#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionDirector.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = 1;
        EstimatedHours = 1.0f;
        bIsCompleted = false;
        Dependencies = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 DinosaurActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        DinosaurActorCount = 0;
        CompletedTasks = 0;
        PendingTasks = 0;
        OverallProgress = 0.0f;
        CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    }
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

/**
 * ProductionDirector - Studio Director's central coordination system
 * Manages agent tasks, tracks production metrics, enforces milestones
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionDirector : public AActor
{
    GENERATED_BODY()

public:
    AProductionDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    float ActorCapLimit;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Task Management")
    void AddAgentTask(const FString& AgentName, const FString& TaskDescription, int32 Priority, float EstimatedHours);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    void CompleteTask(const FString& AgentName, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    int32 GetPendingTaskCount();

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    void EnforceActorCap();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    int32 CountActorsByType(const FString& ActorType);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    bool ValidateMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    void SetCurrentMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestone Management")
    float CalculateOverallProgress();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void AssignCriticalTasks();

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Agent Coordination")
    void ResolveAgentDependencies();

    // Debug and Reporting
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogCurrentStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetAllTasks();

private:
    // Internal tracking
    float LastMetricsUpdate;
    float MetricsUpdateInterval;

    // Helper functions
    void InitializeDefaultTasks();
    void ValidateAgentDependencies();
    bool IsTaskBlocked(const FDir_AgentTask& Task);
    void CleanupExcessActors();
};