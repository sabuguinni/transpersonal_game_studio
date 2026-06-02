#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Dir_ProductionManager.generated.h"

/**
 * Production Phase Enumeration
 * Tracks the current development phase of the game
 */
UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Release         UMETA(DisplayName = "Release")
};

/**
 * Agent Status Enumeration
 * Tracks the current status of each agent in the production pipeline
 */
UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

/**
 * Milestone Status Structure
 * Tracks completion status of major development milestones
 */
USTRUCT(BlueprintType)
struct FDir_MilestoneStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedFeatures;

    FDir_MilestoneStatus()
    {
        MilestoneName = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Agent Task Structure
 * Defines a specific task assigned to an agent
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> DependsOnAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        ExpectedDeliverable = TEXT("");
    }
};

/**
 * Production Metrics Structure
 * Tracks key performance indicators for the development process
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CharacterCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 EnvironmentActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PendingTasks;

    FDir_ProductionMetrics()
    {
        TotalActorsInMap = 0;
        DinosaurCount = 0;
        CharacterCount = 0;
        EnvironmentActors = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        CompletedTasks = 0;
        PendingTasks = 0;
    }
};

/**
 * Production Manager Class
 * Central coordination system for the entire development pipeline
 * Manages agent tasks, milestones, and production metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_ProductionManager : public UObject
{
    GENERATED_BODY()

public:
    UDir_ProductionManager();

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CycleNumber;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneStatus> Milestones;

    // Agent Task Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentTask> AgentTasks;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    // Critical Path Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<int32> CriticalPathAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bMilestone1Blocked;

    // Production Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProduction();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignAgentTask(int32 AgentNumber, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress();

    // Milestone 1 Specific Functions
    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    bool ValidateWalkAroundPrototype();

    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    TArray<FString> GetMilestone1Requirements();

    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    void CheckCriticalPath();

private:
    void SetupMilestone1();
    void SetupAgentDependencies();
    void CalculateCriticalPath();
};