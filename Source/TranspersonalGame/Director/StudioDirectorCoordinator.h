#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "StudioDirectorCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"), 
    Completed   UMETA(DisplayName = "Completed"),
    Error       UMETA(DisplayName = "Error"),
    Blocked     UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime Deadline;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        CycleID = TEXT("");
        StartTime = FDateTime::Now();
        Deadline = FDateTime::Now() + FTimespan::FromHours(1);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 FilesCreated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActorsSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    FString CurrentMilestone;

    FDir_ProductionMetrics()
    {
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
        ActorsSpawned = 0;
        CompletionPercentage = 0.0f;
        CurrentMilestone = TEXT("Milestone 1 - Walk Around");
    }
};

/**
 * Studio Director Coordinator - Central command system for managing all 19 production agents
 * Tracks progress, assigns tasks, monitors deadlines, and ensures milestone completion
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorCoordinator : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === AGENT COORDINATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics Metrics;

    // === MILESTONE TRACKING ===

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, float Priority);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsAgentAvailable(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AdvanceToNextPhase();

    // === PRIORITY TASKS (From Brain Memory) ===

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Priority Tasks")
    void ExecutePriorityTask_PlayerController();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Priority Tasks")
    void ExecutePriorityTask_SurvivalComponent();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Priority Tasks")
    void ExecutePriorityTask_TRexAI();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Priority Tasks")
    void ExecutePriorityTask_CleanupOrphans();

    // === PRODUCTION MONITORING ===

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    FString GetProductionStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Monitoring")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_AgentStatus Status);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    float LastUpdateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Internal")
    FString CurrentCycleID;

    void InitializeProductionPipeline();
    void CheckTaskDeadlines();
    void LogProductionStatus();
};