#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    ContentCreation UMETA(DisplayName = "Content Creation"),
    Polish          UMETA(DisplayName = "Polish & QA"),
    Release         UMETA(DisplayName = "Release Ready")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        TaskDescription = TEXT("");
        RequiredPhase = EDir_ProductionPhase::PreProduction;
        bIsCompleted = false;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bMapSaved;

    FDir_ProductionMetrics()
    {
        TotalActorsSpawned = 0;
        DinosaurCount = 0;
        EnvironmentAssets = 0;
        FrameRate = 60.0f;
        bMapSaved = false;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline and ensures milestone delivery
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_ProductionPhase Phase, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteAgentTask(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsPhaseComplete(EDir_ProductionPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Metrics")
    float GetOverallProgress() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Coordination")
    void ReportAgentStatus(int32 AgentNumber, const FString& StatusMessage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoAdvancePhases;

private:
    float LastMetricsUpdate;
    TMap<int32, FString> AgentStatusMap;

    void ValidateProductionState();
    void CheckMilestoneCompletion();
    void LogProductionStatus() const;
};

#include "ProductionCoordinator.generated.h"