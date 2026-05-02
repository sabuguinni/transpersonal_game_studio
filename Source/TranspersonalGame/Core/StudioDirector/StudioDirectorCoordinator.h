#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"), 
    Completed UMETA(DisplayName = "Completed"),
    Blocked UMETA(DisplayName = "Blocked"),
    Failed UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning UMETA(DisplayName = "Planning"),
    Foundation UMETA(DisplayName = "Foundation"),
    Implementation UMETA(DisplayName = "Implementation"),
    Integration UMETA(DisplayName = "Integration"),
    Testing UMETA(DisplayName = "Testing"),
    Complete UMETA(DisplayName = "Complete")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedOutput;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        ExpectedOutput = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestonePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingIssues;

    FDir_MilestoneProgress()
    {
        MilestoneName = TEXT("");
        CurrentPhase = EDir_MilestonePhase::Planning;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director Coordinator - Central coordination system for all 19 agents
 * Manages task distribution, milestone tracking, and production pipeline
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UStudioDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === MILESTONE 1 COORDINATION ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeMilestone1Tasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& StatusMessage);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID);

    // === PRODUCTION PIPELINE ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateMinPlayableMapState();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportAgentDeliverable(int32 AgentID, const FString& DeliverableName, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockPipelineOnIssue(const FString& IssueDescription, int32 BlockingAgentID);

    // === QUALITY GATES ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void GenerateProductionReport();

    // === AGENT COMMUNICATION ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BroadcastToAllAgents(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SendTaskToAgent(int32 AgentID, const FDir_AgentTask& Task);

protected:
    // === MILESTONE TRACKING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    FDir_MilestoneProgress CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentTask> AgentTaskQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    TMap<int32, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ProductionLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    bool bMilestone1Active;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination", meta = (AllowPrivateAccess = "true"))
    float CycleStartTime;

private:
    // === INTERNAL COORDINATION ===
    void SetupAgentDependencies();
    void CheckTaskDependencies();
    void UpdateMilestoneProgress();
    void LogProductionEvent(const FString& Event);
    
    // === MILESTONE 1 SPECIFIC ===
    void CreateTerrainTasks();
    void CreateCharacterTasks();
    void CreateDinosaurTasks();
    void CreateLightingTasks();
    void CreateHUDTasks();
};