#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "DirectorProductionManager.generated.h"

/**
 * Production status tracking for agent deliverables
 */
UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Development     UMETA(DisplayName = "Development"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration"),
    Complete        UMETA(DisplayName = "Complete")
};

/**
 * Agent task priority levels
 */
UENUM(BlueprintType)
enum class EDir_TaskPriority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High           UMETA(DisplayName = "High"),
    Medium         UMETA(DisplayName = "Medium"),
    Low            UMETA(DisplayName = "Low")
};

/**
 * Individual agent task tracking
 */
USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        TaskName = TEXT("");
        AgentName = TEXT("");
        Priority = EDir_TaskPriority::Medium;
        Phase = EDir_ProductionPhase::Planning;
        Description = TEXT("");
        CompletionPercentage = 0.0f;
        CreatedTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromDays(1);
    }
};

/**
 * Production milestone tracking
 */
USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsBlocking;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        CurrentPhase = EDir_ProductionPhase::Planning;
        TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
        bIsBlocking = false;
    }
};

/**
 * Director Production Manager - Coordinates all agent workflows and tracks deliverables
 * Ensures the 19-agent pipeline produces tangible game content each cycle
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADirectorProductionManager : public AActor
{
    GENERATED_BODY()

public:
    ADirectorProductionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Production tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float DailyBudgetUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float DailyBudgetLimit;

public:
    virtual void Tick(float DeltaTime) override;

    // Task management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateAgentTask(const FString& TaskName, const FString& AgentName, 
                        EDir_TaskPriority Priority, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateTaskProgress(const FString& TaskName, float CompletionPercentage);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteTask(const FString& TaskName, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByAgent(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_TaskPriority Priority);

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& MilestoneName, const FString& Description,
                        const TArray<FString>& RequiredDeliverables, FDateTime TargetDate);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, const TArray<FString>& CompletedDeliverables);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName);

    // Production monitoring
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetCriticalBlockers();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void LogAgentDeliverable(const FString& AgentName, const FString& DeliverableType, 
                            const FString& Description);

    // Quality gates
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetMissingPlayableElements();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void TriggerEmergencyRecovery(const FString& FailureReason);

private:
    // Internal tracking
    UPROPERTY()
    TMap<FString, int32> AgentDeliverableCount;

    UPROPERTY()
    TArray<FString> CycleLog;

    void InitializeCoreMilestones();
    void ValidateAgentOutputs();
    void UpdateProductionMetrics();
};