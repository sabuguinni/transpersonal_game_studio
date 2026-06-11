#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordination System
 * Tracks agent deliverables, build status, and production milestones
 * Ensures all 19 agents work toward unified playable prototype
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    Prototype,
    VerticalSlice,
    Production,
    Polish,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Failed
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    FString CurrentTask;

    UPROPERTY(BlueprintReadOnly)
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(BlueprintReadOnly)
    float Progress = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    FString LastDeliverable;

    UPROPERTY(BlueprintReadOnly)
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalCycles = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CompletedTasks = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveAgents = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 BlockedAgents = 0;

    UPROPERTY(BlueprintReadOnly)
    float OverallProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::PreProduction;

    UPROPERTY(BlueprintReadOnly)
    FString CurrentMilestone;
};

/**
 * Production Coordinator - Studio Director's main coordination system
 * Manages the 19-agent pipeline and tracks deliverables
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UProductionCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgent(int32 AgentID, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus Status, const FString& CurrentTask = "");

    UFUNCTION(BlueprintCallable, Category = "Production")
    void RecordAgentDeliverable(int32 AgentID, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_AgentTask GetAgentStatus(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetAllAgentStatuses() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceProductionPhase(EDir_ProductionPhase NewPhase, const FString& Milestone);

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetCurrentMilestone(const FString& Milestone);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& Milestone) const;

    // Build Status
    UFUNCTION(BlueprintCallable, Category = "Production")
    void RecordBuildStatus(bool bSuccess, const FString& BuildLog = "");

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsLastBuildSuccessful() const { return bLastBuildSuccessful; }

    // Debug and Reporting
    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Production", CallInEditor = true)
    void GenerateProductionReport();

protected:
    UPROPERTY()
    TMap<int32, FDir_AgentTask> AgentTasks;

    UPROPERTY()
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY()
    TArray<FString> CompletedMilestones;

    UPROPERTY()
    bool bLastBuildSuccessful = true;

    UPROPERTY()
    FString LastBuildLog;

private:
    void UpdateProductionMetrics();
    void InitializeAgentRegistry();
};