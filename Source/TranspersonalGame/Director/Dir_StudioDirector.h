#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Dir_StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction   UMETA(DisplayName = "Pre-Production"),
    MinPlayable     UMETA(DisplayName = "Minimum Playable"),
    VerticalSlice   UMETA(DisplayName = "Vertical Slice"),
    Alpha           UMETA(DisplayName = "Alpha"),
    Beta            UMETA(DisplayName = "Beta"),
    Gold            UMETA(DisplayName = "Gold")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString CurrentTask = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ProgressPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString BlockingIssue = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime LastUpdate;

    FDir_AgentTask()
    {
        LastUpdate = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime CompletedDate;
};

/**
 * Studio Director - Agent #01
 * Coordinates all 19 agents, tracks production milestones, and ensures project vision alignment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === PRODUCTION COORDINATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase = EDir_ProductionPhase::MinPlayable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentCycleID = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CycleNumber = 0;

    // === AGENT MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, const FString& TaskDescription);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void AddAgentDeliverable(int32 AgentID, const FString& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Director")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    TArray<int32> GetBlockedAgents() const;

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Director")
    void InitializeProductionMilestones();

    UFUNCTION(BlueprintCallable, Category = "Director")
    void UpdateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Director")
    bool IsMilestoneComplete(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    float GetOverallProgress() const;

    // === COORDINATION FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Director")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Director")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Director")
    FString GenerateProductionReport() const;

    UFUNCTION(BlueprintCallable, Category = "Director")
    void LogProductionStatus() const;

    // === DEBUG & VISUALIZATION ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintAllAgentStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugResetAllAgents();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugSimulateAgentProgress();

private:
    // Internal tracking
    UPROPERTY()
    float LastUpdateTime = 0.0f;

    UPROPERTY()
    bool bIsInitialized = false;

    // Helper functions
    void ValidateAgentChain() const;
    void CheckMilestoneDependencies();
    int32 GetAgentIndex(int32 AgentID) const;
};