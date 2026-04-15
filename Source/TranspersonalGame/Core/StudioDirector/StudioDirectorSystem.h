#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "../../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director Agent #01 - Central Coordination System
 * 
 * Manages the 19-agent production pipeline for Transpersonal Game Studio.
 * Coordinates task distribution, dependency tracking, and quality gates.
 * Ensures Miguel's creative vision reaches the final product intact.
 */

UENUM(BlueprintType)
enum class EDir_AgentType : uint8
{
    StudioDirector = 0,
    EngineArchitect = 1,
    CoreSystems = 2,
    Performance = 3,
    WorldGenerator = 4,
    EnvironmentArtist = 5,
    Architecture = 6,
    Lighting = 7,
    CharacterArtist = 8,
    Animation = 9,
    NPCBehavior = 10,
    CombatAI = 11,
    CrowdSimulation = 12,
    QuestDesigner = 13,
    Narrative = 14,
    Audio = 15,
    VFX = 16,
    QATesting = 17,
    Integration = 18
};

UENUM(BlueprintType)
enum class EDir_TaskStatus : uint8
{
    NotStarted,
    InProgress,
    Blocked,
    Completed,
    Failed,
    RequiresApproval
};

UENUM(BlueprintType)
enum class EDir_Priority : uint8
{
    Critical,
    High,
    Medium,
    Low
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentType AssignedAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_TaskStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime CreatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FDateTime DueTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString Notes;

    FDir_AgentTask()
    {
        TaskID = "";
        Description = "";
        AssignedAgent = EDir_AgentType::StudioDirector;
        Status = EDir_TaskStatus::NotStarted;
        Priority = EDir_Priority::Medium;
        EstimatedHours = 0.0f;
        ActualHours = 0.0f;
        CreatedTime = FDateTime::Now();
        DueTime = FDateTime::Now();
        Notes = "";
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    int32 CycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString CycleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    TArray<FDir_AgentTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString MiguelDirective;

    FDir_ProductionCycle()
    {
        CycleNumber = 0;
        CycleName = "";
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        bIsActive = false;
        MiguelDirective = "";
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
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FDateTime LastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> CompletedDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> BlockingIssues;

    FDir_AgentStatus()
    {
        AgentType = EDir_AgentType::StudioDirector;
        AgentName = "";
        bIsActive = false;
        CurrentTask = "";
        ProgressPercentage = 0.0f;
        LastUpdate = FDateTime::Now();
    }
};

/**
 * Studio Director World Subsystem
 * Manages the entire 19-agent production pipeline
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDir_StudioDirectorSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Production Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartProductionCycle(int32 CycleNumber, const FString& CycleName, const FString& MiguelDirective);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EndProductionCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionCycle GetCurrentCycle() const;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString CreateTask(const FString& Description, EDir_AgentType AssignedAgent, EDir_Priority Priority, float EstimatedHours);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool UpdateTaskStatus(const FString& TaskID, EDir_TaskStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool AddTaskDependency(const FString& TaskID, const FString& DependencyTaskID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(EDir_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetBlockedTasks() const;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(EDir_AgentType AgentType, const FString& CurrentTask, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentStatus GetAgentStatus(EDir_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentStatus> GetAllAgentStatuses() const;

    // Quality Gates
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(EDir_AgentType AgentType) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsArchitectureApproved() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsGameBibleApproved() const;

    // Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    EDir_AgentType GetNextAgentInChain(EDir_AgentType CurrentAgent) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<EDir_AgentType> GetDependentAgents(EDir_AgentType AgentType) const;

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProgressReport() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateBlockageReport() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void LogCurrentStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    FDir_ProductionCycle CurrentProductionCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TArray<FDir_AgentStatus> AgentStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bArchitectureApproved;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bGameBibleApproved;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bQABlocked;

private:
    void InitializeAgentStatuses();
    bool CheckTaskDependencies(const FDir_AgentTask& Task) const;
    FString GetAgentName(EDir_AgentType AgentType) const;
};

/**
 * Studio Director Component for Game Mode
 * Provides easy access to studio coordination from game mode
 */
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_StudioDirectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_StudioDirectorComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    UDir_StudioDirectorSubsystem* GetStudioDirectorSubsystem() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeCycle021();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void CreateCycle021Tasks();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director")
    bool bAutoInitialize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Studio Director")
    int32 TargetCycleNumber;
};