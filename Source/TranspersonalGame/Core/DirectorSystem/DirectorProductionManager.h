#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "DirectorProductionManager.generated.h"

// Production cycle tracking data
USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CycleNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CompletedTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> ActiveAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CompletionPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bCycleCompleted;

    FDir_ProductionCycle()
    {
        CycleNumber = 0;
        CompletionPercentage = 0.0f;
        bCycleCompleted = false;
        StartTime = FDateTime::Now();
        EndTime = FDateTime::MinValue();
    }
};

// Agent status tracking
USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentID;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    float TaskProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    FDateTime LastUpdate;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Agent")
    TArray<FString> CompletedOutputs;

    FDir_AgentStatus()
    {
        AgentNumber = 0;
        bIsActive = false;
        TaskProgress = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

// Production metrics and KPIs
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CompletedCycles;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageCycleTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalUE5Commands;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    TArray<FString> CriticalIssues;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        CompletedCycles = 0;
        AverageCycleTime = 0.0f;
        TotalFilesCreated = 0;
        TotalUE5Commands = 0;
        OverallProgress = 0.0f;
    }
};

/**
 * DirectorProductionManager - Central production coordination system
 * Manages the complete 19-agent production pipeline for Transpersonal Game Studio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorProductionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDirectorProductionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production cycle management
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void StartNewProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor)
    void CompleteCurrentCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionCycle GetCurrentCycle() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_ProductionCycle> GetCycleHistory() const;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Agents", CallInEditor)
    void RegisterAgent(const FString& AgentID, const FString& AgentName, int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void UpdateAgentStatus(const FString& AgentID, const FString& CurrentTask, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    FDir_AgentStatus GetAgentStatus(const FString& AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FDir_AgentStatus> GetAllAgentStatuses() const;

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Metrics")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Metrics", CallInEditor)
    void RecordFileCreation(const FString& FilePath, const FString& AgentID);

    UFUNCTION(BlueprintCallable, Category = "Metrics", CallInEditor)
    void RecordUE5Command(const FString& CommandType, const FString& AgentID);

    // Quality assurance integration
    UFUNCTION(BlueprintCallable, Category = "QA")
    void ReportIssue(const FString& IssueDescription, const FString& AgentID, bool bIsCritical);

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FString> GetCriticalIssues() const;

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ClearResolvedIssues();

    // Director decision support
    UFUNCTION(BlueprintCallable, Category = "Director", CallInEditor)
    void GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool ShouldBlockProduction() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    FString GetNextRecommendedAction() const;

protected:
    // Current production state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FDir_ProductionCycle CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FDir_ProductionCycle> CycleHistory;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, FDir_AgentStatus> AgentStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FDir_ProductionMetrics Metrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxCycleHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float CriticalIssueThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bAutoGenerateReports;

private:
    void InitializeAgentChain();
    void UpdateMetrics();
    void SaveProductionState();
    void LoadProductionState();
};