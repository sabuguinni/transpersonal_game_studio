#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "DirectorTaskManager.generated.h"

UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> RequiredFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> UE5Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedTimeMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime AssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletedTime;

    FDir_AgentTask()
    {
        Priority = EDir_TaskPriority::Medium;
        Status = EDir_TaskStatus::Pending;
        EstimatedTimeMinutes = 10.0f;
        AssignedTime = FDateTime::Now();
        CompletedTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 HeaderFilesCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CppFilesCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float ImplementationRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bCompilationSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FDateTime CycleEndTime;

    FDir_CycleMetrics()
    {
        CycleNumber = 0;
        HeaderFilesCount = 0;
        CppFilesCount = 0;
        ImplementationRatio = 0.0f;
        bCompilationSuccessful = false;
        bPlayablePrototypeReady = false;
        ActorsInLevel = 0;
        CycleStartTime = FDateTime::Now();
        CycleEndTime = FDateTime::MinValue();
    }
};

/**
 * Studio Director Task Manager
 * Coordinates all 19 agents in the production pipeline
 * Tracks progress toward playable prototype milestone
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorTaskManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDirectorTaskManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateTaskStatus(const FString& AgentName, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_TaskPriority Priority) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Director")
    void StartNewCycle(int32 CycleNumber);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void EndCurrentCycle();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateCycleMetrics(const FDir_CycleMetrics& Metrics);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_CycleMetrics GetCurrentCycleMetrics() const;

    // Playable Prototype Assessment
    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsPlayablePrototypeReady() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<FString> GetMissingPrototypeFeatures() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    float GetPrototypeCompletionPercentage() const;

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Director")
    FString GetNextAgentInPipeline(const FString& CurrentAgent) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool CanAgentProceed(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UnblockAgent(const FString& AgentName);

protected:
    // Task Storage
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<FString, TArray<FDir_AgentTask>> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TMap<FString, FString> BlockedAgents;

    // Cycle Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    FDir_CycleMetrics CurrentCycle;

    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FDir_CycleMetrics> CycleHistory;

    // Agent Pipeline Order
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FString> AgentPipelineOrder;

    // Prototype Requirements
    UPROPERTY(BlueprintReadOnly, Category = "Director")
    TArray<FString> PrototypeRequirements;

private:
    void InitializeAgentPipeline();
    void InitializePrototypeRequirements();
    void LogTaskAssignment(const FString& AgentName, const FDir_AgentTask& Task);
    void CheckCriticalPath();
};