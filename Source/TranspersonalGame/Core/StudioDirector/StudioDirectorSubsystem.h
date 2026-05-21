#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction    UMETA(DisplayName = "Pre-Production"),
    Production       UMETA(DisplayName = "Production"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Error           UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime CompletedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 SpawnedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 PopulatedBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bPlayablePrototypeReady;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedTasks = 0;
        ActiveAgents = 0;
        OverallProgress = 0.0f;
        SpawnedDinosaurs = 0;
        PopulatedBiomes = 0;
        bPlayablePrototypeReady = false;
    }
};

/**
 * Studio Director Subsystem - Coordinates all 19 AI agents and tracks production progress
 * Ensures proper agent sequencing, dependency management, and milestone tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentNumber, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentNumber) const;

    // Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetProductionPhase(EDir_ProductionPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentProductionPhase() const { return CurrentPhase; }

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateDinosaurCount(int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeCount(int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetPlayablePrototypeStatus(bool bReady);

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMilestone1_WalkAround() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDependencies(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void GenerateProductionReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FDateTime CycleStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FString CurrentCycleId;

private:
    void InitializeAgentTasks();
    void ValidateAgentSequence();
    bool CheckMilestone1Requirements() const;
    void LogProductionStatus() const;
};