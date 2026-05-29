#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Director_StudioCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldGeneration,
    EnvironmentArt,
    CharacterSystems,
    GameplayMechanics,
    Polish,
    Testing,
    Release
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Blocked,
    Complete,
    Error
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

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
    FDateTime EstimatedCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Phase = EDir_ProductionPhase::PreProduction;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_StudioCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_StudioCoordinator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Production Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AssignTaskToAgent(const FString& AgentName, const FDir_AgentTask& Task);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, const FString& TaskID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(const FString& AgentName, const FString& Reason);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& MilestoneName, EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetMilestoneProgress(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Critical Path Analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetCriticalPathAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void IdentifyBottlenecks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RebalanceWorkload();

    // Quality Assurance Integration
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerQAValidation();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleQABlockage(const FString& BlockageReason);

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitiateBuildProcess();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateBuildIntegrity();

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ActivateEmergencyProtocol(const FString& ProtocolType);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RollbackToLastStableBuild();

    // Reporting and Analytics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogAgentPerformance(const FString& AgentName, float PerformanceScore);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Studio Director")
    EDir_ProductionPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveTasks() const { return ActiveTasks; }

    UFUNCTION(BlueprintPure, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> GetMilestones() const { return ProductionMilestones; }

protected:
    // Core Production State
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TMap<FString, EDir_AgentStatus> AgentStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> CriticalPathAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bQABlockActive;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString LastStableBuildVersion;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Analytics")
    TMap<FString, float> AgentPerformanceScores;

    UPROPERTY(BlueprintReadOnly, Category = "Analytics")
    int32 CompletedTasksCount;

    UPROPERTY(BlueprintReadOnly, Category = "Analytics")
    int32 BlockedTasksCount;

private:
    // Internal Management
    void SetupAgentHierarchy();
    void ValidateTaskDependencies();
    void UpdateCriticalPath();
    void ProcessAgentQueue();
    
    // Emergency Handlers
    void HandleCriticalFailure();
    void ExecuteRollbackProcedure();
    
    // Validation
    bool ValidateAgentCapability(const FString& AgentName, const FString& TaskType);
    bool CheckTaskPrerequisites(const FDir_AgentTask& Task);
};