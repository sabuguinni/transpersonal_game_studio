#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Production      UMETA(DisplayName = "Production"),
    Polish          UMETA(DisplayName = "Polish"),
    Release         UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
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
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<int32> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

/**
 * Studio Director's Production Coordination System
 * Manages all 19 AI agents and tracks production milestones
 * Ensures proper task sequencing and dependency management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Production Phase Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float CycleDuration;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agents")
    TMap<int32, EDir_AgentStatus> AgentStatusMap;

    // Milestone Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    // Production Metrics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalUE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageTaskCompletionTime;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<int32> GetAvailableAgents();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void ResolveAgentDependencies(int32 AgentID);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void UpdateMilestoneProgress(const FString& MilestoneName, float ProgressPercentage);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetOverallProductionProgress();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void GenerateProductionReport();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugSpawnMilestoneMarkers();

private:
    // Internal state management
    void UpdateProductionMetrics();
    void CheckForBlockedTasks();
    void OptimizeTaskScheduling();
    
    // Agent communication
    void SendTaskToAgent(int32 AgentID, const FDir_AgentTask& Task);
    void ReceiveAgentUpdate(int32 AgentID, const FString& UpdateMessage);
    
    // Milestone validation
    bool ValidateMilestoneRequirements(const FDir_ProductionMilestone& Milestone);
    void TriggerMilestoneCompletion(const FString& MilestoneName);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Timer handles for production cycles
    FTimerHandle ProductionCycleTimer;
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle AgentStatusTimer;
};