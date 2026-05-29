#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Director_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction,
    CoreSystems,
    WorldGeneration,
    CharacterImplementation,
    GameplayMechanics,
    Polish,
    Testing,
    Complete
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
struct FDir_AgentTaskInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Outputs;

    FDir_AgentTaskInfo()
    {
        AgentID = 0;
        AgentName = TEXT("");
        Status = EDir_AgentStatus::Idle;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Deliverables;

    FDir_MilestoneInfo()
    {
        MilestoneName = TEXT("");
        Phase = EDir_ProductionPhase::PreProduction;
        bIsComplete = false;
        TargetDate = FDateTime::Now();
    }
};

/**
 * Production Coordinator for managing the 19-agent development pipeline
 * Tracks agent status, dependencies, milestones, and critical path
 */
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTaskInfo> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_MilestoneInfo> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool CanAgentProceed(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<int32> GetBlockedAgents();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Production")
    FString GetProductionStatusReport();

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Production")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool IsMilestoneComplete(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetPendingMilestones();

    // Agent Coordination
    UFUNCTION(BlueprintCallable, Category = "Production")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void RegisterAgentOutput(int32 AgentID, const FString& OutputDescription);

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetAgentOutputs(int32 AgentID);

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidatePhaseCompletion(EDir_ProductionPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Production")
    void TriggerQualityGate(EDir_ProductionPhase Phase);

private:
    void SetupAgentDependencies();
    void SetupMilestones();
    void UpdateCriticalPath();
    bool CheckDependenciesMet(int32 AgentID);
    void CalculateOverallProgress();
};

#include "Director_ProductionCoordinator.generated.h"