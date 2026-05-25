#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    GameplayLoop    UMETA(DisplayName = "Gameplay Loop"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Release         UMETA(DisplayName = "Release Ready")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase RequiredPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority = 1.0f;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        RequiredPhase = EDir_ProductionPhase::PreProduction;
        bIsBlocking = false;
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockingTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        BlockingTasks = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline and ensures proper task sequencing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AddAgentTask(const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentID, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetPendingTasksForAgent(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase();

    // Phase Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentProductionPhase; }

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPhaseBlocked() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgentCompletion(int32 AgentID, const FString& Output);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetNextAgentInChain(int32 CurrentAgentID) const;

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void ResetProductionPipeline();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentProductionPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TMap<int32, FString> AgentOutputs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<int32> BlockedAgents;

private:
    void SetupAgentDependencies();
    void ValidateTaskSequence();
    bool CheckDependenciesComplete(int32 AgentID) const;
    void UpdateProductionMetrics();
};