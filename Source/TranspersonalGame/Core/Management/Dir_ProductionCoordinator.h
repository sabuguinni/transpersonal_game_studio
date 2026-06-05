#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Dir_ProductionCoordinator.generated.h"

UENUM(BlueprintType)
enum class EDir_ProductionMilestone : uint8
{
    None = 0,
    WalkAround = 1,          // Basic character movement and camera
    BasicSurvival = 2,       // HUD, health, hunger, thirst systems
    DinosaurEncounters = 3,  // AI dinosaurs with basic behavior
    CombatSystem = 4,        // Combat mechanics and weapons
    WorldGeneration = 5,     // Procedural terrain and biomes
    FullPrototype = 6        // Complete playable experience
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle = 0,
    Working = 1,
    Completed = 2,
    Blocked = 3,
    Failed = 4
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
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_ProductionMilestone RequiredMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> DependsOnAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FDateTime EstimatedCompletion;

    FDir_AgentTask()
    {
        AgentID = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        RequiredMilestone = EDir_ProductionMilestone::None;
        Status = EDir_AgentStatus::Idle;
        Priority = 1.0f;
        StartTime = FDateTime::Now();
        EstimatedCompletion = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    EDir_ProductionMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime ProjectStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastUpdate;

    FDir_ProductionMetrics()
    {
        TotalAgents = 19;
        ActiveAgents = 0;
        CompletedTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        CurrentMilestone = EDir_ProductionMilestone::WalkAround;
        ProjectStartTime = FDateTime::Now();
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Studio Director Production Coordinator
 * Manages the 19-agent production pipeline for Transpersonal Game Studio
 * Coordinates tasks, tracks milestones, and ensures smooth workflow between agents
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UDir_ProductionCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_ProductionCoordinator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Production Pipeline Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    bool bAutoProgressMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Pipeline")
    float TaskUpdateInterval;

private:
    float LastUpdateTime;
    int32 CurrentCycleID;

public:
    // Core Coordination Functions
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionPipeline();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_ProductionMilestone RequiredMilestone, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(int32 AgentID) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AdvanceToNextMilestone();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForMilestone(EDir_ProductionMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockAgent(int32 AgentID);

    // Milestone Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMilestoneCompletion(EDir_ProductionMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculateMilestoneProgress(EDir_ProductionMilestone Milestone) const;

    // Agent Communication
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SendMessageToAgent(int32 AgentID, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetMessagesForAgent(int32 AgentID) const;

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void PrintProductionStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void ResetProductionPipeline();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void ForceAdvanceMilestone();

protected:
    // Internal Pipeline Management
    void UpdateProductionMetrics();
    void ProcessAgentDependencies();
    void CheckMilestoneCompletion();
    void HandleBlockedAgents();
    
    // Agent-specific task definitions
    void SetupAgentTasks();
    void CreateEngineArchitectTasks();
    void CreateCoreSystemsTasks();
    void CreatePerformanceTasks();
    void CreateWorldGenerationTasks();
    void CreateEnvironmentTasks();
    void CreateCharacterTasks();
    void CreateAnimationTasks();
    void CreateAITasks();
    void CreateQuestTasks();
    void CreateNarrativeTasks();
    void CreateAudioTasks();
    void CreateVFXTasks();
    void CreateQATasks();
    void CreateIntegrationTasks();

private:
    TMap<int32, TArray<FString>> AgentMessages;
    TMap<int32, FString> AgentBlockReasons;
};