#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "ProductionCoordinator.generated.h"

/**
 * Studio Director's production coordination system
 * Manages agent task distribution and milestone tracking
 */

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    Prototype      UMETA(DisplayName = "Prototype"),
    Production     UMETA(DisplayName = "Production"),
    Polish         UMETA(DisplayName = "Polish"),
    Release        UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Working        UMETA(DisplayName = "Working"),
    Blocked        UMETA(DisplayName = "Blocked"),
    Complete       UMETA(DisplayName = "Complete"),
    Failed         UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime DeadlineTime;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now() + FTimespan::FromHours(24);
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
        TargetDate = FDateTime::Now() + FTimespan::FromDays(7);
    }
};

/**
 * Production Coordinator Component
 * Attached to GameMode to manage production workflow
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::Prototype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float ProductionBudget = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float BudgetUsed = 0.0f;

public:
    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FDir_AgentTask> GetTasksForAgent(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool AreAgentDependenciesMet(int32 AgentID) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateMilestoneProgress(const FString& MilestoneName, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    // Production Analytics
    UFUNCTION(BlueprintCallable, Category = "Production")
    float GetOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionReport() const;

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    int32 GetCurrentCycle() const { return CurrentCycle; }

private:
    void InitializeDefaultMilestones();
    void UpdateTaskPriorities();
    void CheckMilestoneCompletion();
    bool ValidateAgentChain() const;
};