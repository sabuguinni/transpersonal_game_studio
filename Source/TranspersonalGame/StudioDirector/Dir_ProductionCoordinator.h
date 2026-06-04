#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    Planning UMETA(DisplayName = "Planning"),
    Development UMETA(DisplayName = "Development"), 
    Testing UMETA(DisplayName = "Testing"),
    Integration UMETA(DisplayName = "Integration"),
    Complete UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"),
    Blocked UMETA(DisplayName = "Blocked"),
    Complete UMETA(DisplayName = "Complete"),
    Error UMETA(DisplayName = "Error")
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
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        Dependencies = TEXT("");
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
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCriticalPath;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Phase = EDir_ProductionPhase::Planning;
        CompletionPercentage = 0.0f;
        bIsCriticalPath = false;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Core production coordination functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionSystem();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(const FString& AgentName, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void CreateNewTask(const FString& AgentName, const FString& Description, const FString& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void ValidateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GenerateProductionReport();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CheckCriticalPathBlocked();

    // Agent coordination functions
    UFUNCTION(BlueprintCallable, Category = "Agents")
    void DispatchTaskToAgent(int32 AgentNumber, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Agents")
    TArray<FString> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Agents")
    void ResolveAgentConflict(const FString& Agent1, const FString& Agent2, const FString& Resolution);

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CreateMilestone(const FString& Name, EDir_ProductionPhase Phase, bool bCriticalPath);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    float GetMilestoneProgress(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Milestones")
    void CompleteMilestone(const FString& MilestoneName);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProjectProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bProductionSystemActive;

private:
    void UpdateMilestoneProgress();
    void CheckTaskDependencies();
    void LogProductionMetrics();
    FString FormatProgressReport();
};