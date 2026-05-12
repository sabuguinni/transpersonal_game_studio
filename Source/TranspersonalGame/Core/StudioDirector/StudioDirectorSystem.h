#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Central coordination hub for the 19-agent development pipeline
 * Manages task assignment, milestone tracking, and production coordination
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Working         UMETA(DisplayName = "Working"),
    Completed       UMETA(DisplayName = "Completed"),
    Blocked         UMETA(DisplayName = "Blocked"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestoneType : uint8
{
    WalkAround      UMETA(DisplayName = "Walk Around"),
    BasicSurvival   UMETA(DisplayName = "Basic Survival"),
    DinosaurAI      UMETA(DisplayName = "Dinosaur AI"),
    FullGameplay    UMETA(DisplayName = "Full Gameplay"),
    Polish          UMETA(DisplayName = "Polish")
};

UENUM(BlueprintType)
enum class EDir_Priority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float ActualDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentID = 0;
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        Status = EDir_AgentStatus::Idle;
        EstimatedDuration = 0.0f;
        ActualDuration = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    EDir_MilestoneType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<int32> RequiredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime TargetDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime ActualCompletionDate;

    FDir_ProductionMilestone()
    {
        Type = EDir_MilestoneType::WalkAround;
        Description = TEXT("");
        CompletionPercentage = 0.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FDir_CriticalPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    FString PathName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    TArray<int32> AgentSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    float TotalEstimatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    float CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Path")
    FString BlockingReason;

    FDir_CriticalPath()
    {
        PathName = TEXT("");
        TotalEstimatedTime = 0.0f;
        CurrentProgress = 0.0f;
        bIsBlocked = false;
        BlockingReason = TEXT("");
    }
};

/**
 * Studio Director Subsystem - Manages the entire development pipeline
 */
UCLASS()
class TRANSPERSONALGAME_API UStudioDirectorSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, EDir_Priority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(int32 AgentID, const TArray<FString>& Deliverables);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetAllAgentTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentID) const;

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestone(EDir_MilestoneType Type, const FString& Description, const TArray<int32>& RequiredAgents);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMilestoneProgress(EDir_MilestoneType Type, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteMilestone(EDir_MilestoneType Type);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_ProductionMilestone> GetAllMilestones() const;

    // Critical Path Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateCriticalPath(const FString& PathName, const TArray<int32>& AgentSequence);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateCriticalPathProgress(const FString& PathName, float NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockCriticalPath(const FString& PathName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockCriticalPath(const FString& PathName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_CriticalPath> GetAllCriticalPaths() const;

    // Production Analytics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProductionProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetBlockedAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<int32> GetCriticalPathAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateProductionReport() const;

    // Emergency Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerEmergencyMode(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ExitEmergencyMode();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsInEmergencyMode() const;

protected:
    UPROPERTY()
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY()
    TArray<FDir_ProductionMilestone> ProductionMilestones;

    UPROPERTY()
    TArray<FDir_CriticalPath> CriticalPaths;

    UPROPERTY()
    bool bEmergencyMode;

    UPROPERTY()
    FString EmergencyReason;

    UPROPERTY()
    FDateTime ProductionStartTime;

private:
    void InitializeAgentTasks();
    void InitializeProductionMilestones();
    void InitializeCriticalPaths();
    void UpdateDependencies();
    float CalculateMilestoneProgress(const FDir_ProductionMilestone& Milestone) const;
};

/**
 * Studio Director Actor - Visual representation in the world
 */
UCLASS()
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* HubMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* AgentIndicatorRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<class AStaticMeshActor*> AgentStatusActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<class AStaticMeshActor*> MilestoneActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<class AStaticMeshActor*> CriticalPathActors;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateVisualStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateAgentStatusIndicators();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateMilestoneMarkers();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateCriticalPathIndicators();

protected:
    UFUNCTION()
    void UpdateAgentIndicatorColors();

    UFUNCTION()
    void UpdateMilestoneProgress();

    UFUNCTION()
    void UpdateCriticalPathStatus();

private:
    UPROPERTY()
    class UStudioDirectorSubsystem* DirectorSubsystem;

    float LastUpdateTime;
    static constexpr float UpdateInterval = 1.0f;
};

#include "StudioDirectorSystem.generated.h"