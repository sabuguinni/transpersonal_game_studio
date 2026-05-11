#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    CharacterDev    UMETA(DisplayName = "Character Development"),
    GameplayLoop    UMETA(DisplayName = "Gameplay Loop"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Release         UMETA(DisplayName = "Release Ready")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        StartTime = FDateTime::Now();
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Production Coordinator - Studio Director's central command system
 * Manages the 18-agent production pipeline for the Transpersonal Game Studio
 * Tracks tasks, dependencies, metrics, and coordinates agent workflows
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintPure, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    // Agent Task Management
    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void AddAgentTask(const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Tasks")
    void CompleteAgentTask(int32 AgentID, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintPure, Category = "Tasks")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID) const;

    UFUNCTION(BlueprintPure, Category = "Tasks")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    UFUNCTION(BlueprintPure, Category = "Tasks")
    bool CanAgentProceed(int32 AgentID) const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintPure, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Emergency Controls
    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void EmergencyStopProduction();

    UFUNCTION(BlueprintCallable, Category = "Emergency", CallInEditor)
    void ResetProductionPipeline();

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FString> GetBuildErrors() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    int32 GetNextAgentToExecute() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void NotifyAgentStarted(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void NotifyAgentCompleted(int32 AgentID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    bool bProductionActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentExecutingAgent = 1;

    // Agent Names for reference
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Agents")
    TMap<int32, FString> AgentNames;

private:
    void InitializeAgentNames();
    void CheckDependencies();
    void UpdateMetricsFromWorld();
    bool ValidateAgentDeliverables(int32 AgentID) const;
};