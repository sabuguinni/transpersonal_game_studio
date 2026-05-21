#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Timeout     UMETA(DisplayName = "Timeout")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority = 1.0f;

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
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromMinutes(30);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCycles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 FailedTasks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float AverageTaskTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Planning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress = 0.0f;
};

/**
 * Studio Director System - Coordinates all 19 agents in the production pipeline
 * Manages task distribution, monitors progress, and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables);

    // Monitoring and metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsAgentAvailable(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    // Emergency controls
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStopAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RestartFailedTasks();

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnProductionCycleStarted(const FString& CycleID);

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnAgentTaskCompleted(int32 AgentNumber, const FString& TaskDescription);

    UFUNCTION(BlueprintImplementableEvent, Category = "Studio Director")
    void OnProductionPhaseChanged(EDir_ProductionPhase NewPhase);

private:
    // Internal state
    UPROPERTY()
    TMap<int32, FDir_AgentTask> AgentTasks;

    UPROPERTY()
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY()
    FString CurrentCycleID;

    UPROPERTY()
    FDateTime CycleStartTime;

    // Internal functions
    void InitializeAgentSlots();
    void UpdateProductionMetrics();
    bool ValidateAgentNumber(int32 AgentNumber) const;
    void LogProductionEvent(const FString& Event) const;
};