#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Production      UMETA(DisplayName = "Production"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration"),
    Deployment      UMETA(DisplayName = "Deployment")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime DeadlineTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 FailedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float CycleDurationMinutes;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        FailedTasks = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::PreProduction;
        CycleStartTime = FDateTime::Now();
        CycleDurationMinutes = 0.0f;
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 18-agent development pipeline with centralized task coordination
 * Tracks agent status, dependencies, and production metrics
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    bool bProductionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float CycleTimeoutMinutes;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void InitializeProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void CompleteAgentTask(const FString& AgentName, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    bool CheckAgentDependencies(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Management")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Metrics and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    float GetCycleProgress();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Metrics")
    TArray<FString> GetActiveAgents();

    // Emergency Management
    UFUNCTION(BlueprintCallable, Category = "Emergency Management")
    void TriggerEmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Emergency Management")
    void ResetProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Emergency Management")
    bool IsAgentBlocked(const FString& AgentName);

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Production Events")
    void OnCycleStarted(const FString& CycleID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Production Events")
    void OnAgentCompleted(const FString& AgentName);

    UFUNCTION(BlueprintImplementableEvent, Category = "Production Events")
    void OnProductionPhaseChanged(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Production Events")
    void OnEmergencyTriggered(const FString& Reason);

private:
    void UpdateProductionMetrics();
    void CheckForTimeouts();
    void ValidateAgentChain();
    FDir_AgentTask* FindAgentTask(const FString& AgentName);
};