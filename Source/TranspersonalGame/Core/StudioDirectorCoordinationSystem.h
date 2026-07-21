#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "StudioDirectorCoordinationSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle = 0,
    Active = 1,
    Blocked = 2,
    Complete = 3,
    Error = 4
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Architecture = 0,
    CoreSystems = 1,
    WorldGeneration = 2,
    Characters = 3,
    AI = 4,
    Audio = 5,
    Integration = 6,
    Testing = 7
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    FDir_AgentTask()
    {
        TaskName = "";
        Description = "";
        Priority = 0;
        EstimatedDuration = 0.0f;
        Status = EDir_AgentStatus::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EDir_ProductionPhase CurrentPhase;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        CurrentPhase = EDir_ProductionPhase::Architecture;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorCoordinationSystem : public UObject
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinationSystem();

    // Core coordination functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateAgentDependencies(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyProtocol(const FString& Reason);

    // Production monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAgentTasks(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceProductionPhase();

    // Milestone validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // Emergency protocols
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleAgentTimeout(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleBuildFailure();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleQABlocker(const FString& BlockerDescription);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TMap<int32, TArray<FDir_AgentTask>> AgentTaskQueues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TMap<int32, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ProductionLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production", meta = (AllowPrivateAccess = "true"))
    bool bEmergencyMode;

private:
    void InitializeAgentStatuses();
    void CalculateProductionMetrics();
    void LogProductionEvent(const FString& Event);
    bool CheckMilestone1Requirements();
    void SetupAgentDependencies();
};

#include "StudioDirectorCoordinationSystem.generated.h"