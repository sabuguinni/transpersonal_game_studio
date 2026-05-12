#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordination System
 * Manages the 18-agent development pipeline and milestone tracking
 * Ensures systematic delivery of game systems and features
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum class EDir_MilestoneStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Testing         UMETA(DisplayName = "Testing"),
    Complete        UMETA(DisplayName = "Complete"),
    Blocked         UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Deliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> FilesRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString LastUpdate;

    FDir_AgentTask()
    {
        Priority = 0;
        Status = EDir_AgentStatus::Idle;
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_Milestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString TargetCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> BlockingIssues;

    FDir_Milestone()
    {
        Status = EDir_MilestoneStatus::NotStarted;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalCycles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 HeaderFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ImplementationFilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 PlayableFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    FDir_ProductionMetrics()
    {
        TotalCycles = 0;
        HeaderFilesCreated = 0;
        ImplementationFilesCreated = 0;
        PlayableFeatures = 0;
        CompilationErrors = 0;
        OverallProgress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    TArray<FDir_Milestone> ProjectMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Coordination")
    FDir_ProductionMetrics CurrentMetrics;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool CheckAgentDependencies(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateMilestoneProgress(const FString& MilestoneName, float NewPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FString GetNextPriorityAgent();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void CreateMilestone(const FString& Name, const FString& TargetCycle, const TArray<FString>& Requirements);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    bool IsMilestoneComplete(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void ValidateCurrentMilestone();

    // Metrics and Reporting
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void LogCriticalIssue(const FString& Issue, const FString& AffectedAgent);

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void TriggerEmergencyProtocol(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Production Coordination")
    void ResetAgentPipeline();

private:
    // Internal tracking
    UPROPERTY()
    FString CurrentCycleID;

    UPROPERTY()
    float CycleStartTime;

    UPROPERTY()
    TArray<FString> CriticalIssues;

    // Helper functions
    void ValidateAgentDependencies();
    void CheckCompilationStatus();
    void UpdateCycleMetrics();
    bool IsAgentBlocked(const FString& AgentName);
};