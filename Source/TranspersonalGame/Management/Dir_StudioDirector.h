#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Dir_StudioDirector.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionPhase : uint8
{
    PreProduction    UMETA(DisplayName = "Pre-Production"),
    PrototypePhase   UMETA(DisplayName = "Prototype Phase"),
    ProductionPhase  UMETA(DisplayName = "Production Phase"),
    PolishPhase      UMETA(DisplayName = "Polish Phase"),
    ReleaseReady     UMETA(DisplayName = "Release Ready")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        CurrentTask = TEXT("");
        Status = EDir_AgentStatus::Idle;
        ProgressPercentage = 0.0f;
        Priority = TEXT("Normal");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    FDir_ProductionMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsComplete = false;
        CompletionPercentage = 0.0f;
    }
};

/**
 * Studio Director - Master coordinator for all 19 AI agents in the production pipeline
 * Manages production phases, agent tasks, milestones, and overall project vision
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADir_StudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ADir_StudioDirector();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // === PRODUCTION MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EDir_ProductionPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentFocus;

    // === AGENT COORDINATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    int32 ActiveAgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FString> BlockedAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coordination")
    TArray<FString> CompletedTasks;

public:
    virtual void Tick(float DeltaTime) override;

    // === PRODUCTION MANAGEMENT FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, const FString& Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CheckMilestoneCompletion(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceProductionPhase();

    // === COORDINATION FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResolveAgentBlocking(int32 AgentID, const FString& Resolution);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculateOverallProgress();

    // === MILESTONE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(const FString& Name, const FString& Description, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteMilestone(const FString& MilestoneName);

    // === DEBUG AND MONITORING ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetAllAgentTasks();

private:
    void SetupInitialMilestones();
    void ValidateAgentDependencies();
    void UpdateProductionMetrics();
};