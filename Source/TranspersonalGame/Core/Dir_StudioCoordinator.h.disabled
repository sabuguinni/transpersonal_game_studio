#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Dir_StudioCoordinator.generated.h"

/**
 * Studio Director's coordination system for managing agent tasks and production pipeline
 * Tracks milestone progress, agent assignments, and production metrics in real-time
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle,
    Working,
    Completed,
    Blocked,
    Error
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    Milestone1_WalkAround,
    Milestone2_BasicSurvival,
    Milestone3_DinosaurInteraction,
    Milestone4_FullGameplay
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FVector TaskLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        TaskLocation = FVector::ZeroVector;
        EstimatedDuration = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MilestoneProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FString CurrentCycleID;

    FDir_ProductionMetrics()
    {
        MilestoneProgress = 0.0f;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallEfficiency = 0.0f;
        CurrentCycleID = TEXT("UNKNOWN");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDir_StudioCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDir_StudioCoordinator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent task management
    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, float Priority, const FVector& TaskLocation);

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    TArray<FDir_AgentTask> GetActiveAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    FDir_AgentTask GetAgentTask(const FString& AgentName) const;

    // Milestone tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void SetCurrentMilestone(EDir_MilestoneType Milestone);

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    EDir_MilestoneType GetCurrentMilestone() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    float CalculateMilestoneProgress();

    // Production metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    FDir_ProductionMetrics GetProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void UpdateCycleID(const FString& NewCycleID);

    // Level analysis
    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void AnalyzeLevelState();

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    int32 CountActorsOfType(const FString& ActorTypeName);

    // Coordination markers
    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void CreateCoordinationMarkers();

    UFUNCTION(BlueprintCallable, Category = "Studio Coordination")
    void UpdateTaskMarker(const FString& AgentName, const FVector& NewLocation);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Coordination")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Coordination")
    EDir_MilestoneType CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Coordination")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Coordination")
    TMap<FString, AActor*> CoordinationMarkers;

private:
    void InitializeDefaultTasks();
    void CalculateEfficiencyMetrics();
    float GetTaskCompletionWeight(const FString& AgentName) const;
};