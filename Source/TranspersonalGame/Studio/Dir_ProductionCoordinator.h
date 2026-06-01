#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

/**
 * Studio Director Production Coordinator
 * Manages agent coordination and production pipeline
 * Tracks deliverables and ensures milestone completion
 */

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
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
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
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime Deadline;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 0;
        EstimatedHours = 0.0f;
        Deadline = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FDir_AgentTask> RequiredTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        bCompleted = false;
        TargetDate = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_ProductionCoordinator : public AActor
{
    GENERATED_BODY()

public:
    ADir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CoordinatorMesh;

    // Production Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionEfficiency;

public:
    virtual void Tick(float DeltaTime) override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTask(const FString& AgentName, const FString& TaskDescription, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskStatus(const FString& AgentName, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status);

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& Name, const FDateTime& TargetDate);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetProjectProgress();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CoordinateAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void GenerateProductionReport();

    // Critical Production Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void ValidateCurrentBuild();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Production")
    void CreatePlayablePrototype();

protected:
    // Internal coordination logic
    void UpdateProductionMetrics();
    void CheckBlockedTasks();
    void OptimizeAgentWorkflow();
    
    // Critical milestone tracking
    void TrackPrototypeProgress();
    void ValidateAgentDeliverables();
};

#include "Dir_ProductionCoordinator.generated.h"