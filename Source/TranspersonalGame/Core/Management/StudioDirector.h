#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director - Master coordination system for all game agents and production milestones
 * Manages the 19-agent development pipeline and ensures milestone completion
 */

UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    WalkAround          UMETA(DisplayName = "Walk Around"),
    BasicSurvival       UMETA(DisplayName = "Basic Survival"),
    DinosaurEncounters  UMETA(DisplayName = "Dinosaur Encounters"),
    FullGameplay        UMETA(DisplayName = "Full Gameplay")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = "";
        CurrentTask = "";
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_ProductionMilestone MilestoneType = EDir_ProductionMilestone::WalkAround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage = 0.0f;

    FDir_MilestoneData()
    {
        MilestoneType = EDir_ProductionMilestone::WalkAround;
        MilestoneName = "";
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirector : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // === AGENT MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    int32 CurrentCycleNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Management")
    FString CurrentCycleID = "";

    // === MILESTONE TRACKING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    TArray<FDir_MilestoneData> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestones")
    EDir_ProductionMilestone CurrentMilestone = EDir_ProductionMilestone::WalkAround;

    // === PRODUCTION METRICS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalFilesCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalUE5CommandsExecuted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float ProductionEfficiency = 0.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // === AGENT COORDINATION ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, const TArray<FString>& Dependencies);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentNumber) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetBlockedAgents() const;

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeMilestones();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(EDir_ProductionMilestone Milestone, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestoneCompleted(EDir_ProductionMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceToNextMilestone();

    // === PRODUCTION COORDINATION ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateWorldState();

    // === DEBUGGING & MONITORING ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugPrintAgentStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugPrintMilestoneStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void ResetProductionPipeline();

private:
    void SetupDefaultAgents();
    void SetupDefaultMilestones();
    FDir_AgentTask* FindAgentTask(int32 AgentNumber);
    FDir_MilestoneData* FindMilestone(EDir_ProductionMilestone Milestone);
};