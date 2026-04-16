#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director System - Coordinates all 19 agents in the production pipeline
 * Manages task distribution, progress tracking, and quality assurance
 */

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector      UMETA(DisplayName = "Studio Director"),
    EngineArchitect     UMETA(DisplayName = "Engine Architect"),
    CoreSystems         UMETA(DisplayName = "Core Systems"),
    Performance         UMETA(DisplayName = "Performance Optimizer"),
    WorldGeneration     UMETA(DisplayName = "World Generation"),
    Environment         UMETA(DisplayName = "Environment Art"),
    Architecture        UMETA(DisplayName = "Architecture & Interior"),
    Lighting            UMETA(DisplayName = "Lighting & Atmosphere"),
    Characters          UMETA(DisplayName = "Character Artist"),
    Animation           UMETA(DisplayName = "Animation"),
    NPCBehavior         UMETA(DisplayName = "NPC Behavior"),
    CombatAI            UMETA(DisplayName = "Combat & Enemy AI"),
    CrowdSimulation     UMETA(DisplayName = "Crowd & Traffic"),
    QuestDesign         UMETA(DisplayName = "Quest & Mission Design"),
    Narrative           UMETA(DisplayName = "Narrative & Dialogue"),
    Audio               UMETA(DisplayName = "Audio"),
    VFX                 UMETA(DisplayName = "VFX"),
    QA                  UMETA(DisplayName = "QA & Testing"),
    Integration         UMETA(DisplayName = "Integration & Build")
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    Pending             UMETA(DisplayName = "Pending"),
    InProgress          UMETA(DisplayName = "In Progress"),
    Completed           UMETA(DisplayName = "Completed"),
    Blocked             UMETA(DisplayName = "Blocked"),
    Failed              UMETA(DisplayName = "Failed"),
    RequiresApproval    UMETA(DisplayName = "Requires Approval")
};

UENUM(BlueprintType)
enum class EDir_Priority : uint8
{
    Critical            UMETA(DisplayName = "Critical"),
    High                UMETA(DisplayName = "High"),
    Medium              UMETA(DisplayName = "Medium"),
    Low                 UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentType AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DueTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CompletedTime;

    FDir_AgentTask()
    {
        TaskID = "";
        TaskName = "";
        Description = "";
        AssignedAgent = EDir_AgentType::StudioDirector;
        Status = EDir_TaskStatus::Pending;
        Priority = EDir_Priority::Medium;
        EstimatedHours = 0.0f;
        ActualHours = 0.0f;
        CreatedTime = FDateTime::Now();
        DueTime = FDateTime::Now();
        CompletedTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_AgentStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float WorkloadPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 PendingTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastActivity;

    FDir_AgentStatus()
    {
        AgentType = EDir_AgentType::StudioDirector;
        AgentName = "";
        bIsActive = false;
        bIsBlocked = false;
        CurrentTask = "";
        WorkloadPercentage = 0.0f;
        CompletedTasks = 0;
        PendingTasks = 0;
        LastActivity = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 InProgressTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float OverallProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float EstimatedCompletionDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 BlockedAgents;

    FDir_ProductionMetrics()
    {
        TotalTasks = 0;
        CompletedTasks = 0;
        InProgressTasks = 0;
        BlockedTasks = 0;
        OverallProgress = 0.0f;
        EstimatedCompletionDays = 0.0f;
        ActiveAgents = 0;
        BlockedAgents = 0;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DirectorDeskMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* HologramProjector;

    // Agent Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Management")
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Management")
    TArray<FDir_AgentTask> TaskQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FDir_ProductionMetrics ProductionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CurrentCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString CurrentPhase;

    // Task Management Functions
    UFUNCTION(BlueprintCallable, Category = "Task Management")
    FString CreateTask(const FString& TaskName, const FString& Description, 
                      EDir_AgentType AssignedAgent, EDir_Priority Priority, 
                      float EstimatedHours);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    bool AssignTaskToAgent(const FString& TaskID, EDir_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    bool UpdateTaskStatus(const FString& TaskID, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    bool CompleteTask(const FString& TaskID, float ActualHours);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    bool BlockTask(const FString& TaskID, const FString& BlockReason);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    TArray<FDir_AgentTask> GetTasksForAgent(EDir_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Task Management")
    TArray<FDir_AgentTask> GetTasksByStatus(EDir_TaskStatus Status);

    // Agent Management Functions
    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool RegisterAgent(EDir_AgentType AgentType, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool SetAgentActive(EDir_AgentType AgentType, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    bool BlockAgent(EDir_AgentType AgentType, const FString& BlockReason);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    FDir_AgentStatus GetAgentStatus(EDir_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<EDir_AgentType> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Agent Management")
    TArray<EDir_AgentType> GetBlockedAgents();

    // Production Management Functions
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StartNewCycle();

    UFUNCTION(BlueprintCallable, Category = "Production")
    void SetProductionPhase(const FString& PhaseName);

    UFUNCTION(BlueprintCallable, Category = "Production")
    FDir_ProductionMetrics CalculateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Production")
    bool ValidateAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Production")
    TArray<FString> GetProductionReport();

    // Quality Assurance Functions
    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    bool ValidateTaskDependencies(const FString& TaskID);

    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    TArray<FString> GetBlockedTaskReasons();

    UFUNCTION(BlueprintCallable, Category = "Quality Assurance")
    bool CanAgentProceed(EDir_AgentType AgentType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GenerateTaskID();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetAgentName(EDir_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    EDir_AgentType GetNextAgentInChain(EDir_AgentType CurrentAgent);

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintAllTasks();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintAgentStatuses();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugValidateProductionChain();

private:
    // Internal helper functions
    void InitializeAgents();
    void UpdateProductionMetrics();
    FDir_AgentTask* FindTaskByID(const FString& TaskID);
    FDir_AgentStatus* FindAgentStatus(EDir_AgentType AgentType);
    bool ValidateTaskChain();
    void ProcessTaskDependencies();
    
    // Internal state
    int32 NextTaskIDCounter;
    float LastMetricsUpdate;
    bool bProductionActive;
};