#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "DirectorCoordinator.generated.h"

/**
 * Studio Director Coordination System
 * Manages the 19-agent production pipeline and ensures milestone delivery
 * Tracks agent outputs and coordinates cross-agent dependencies
 */

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Completed   UMETA(DisplayName = "Completed"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_MilestonePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Foundation      UMETA(DisplayName = "Foundation"),
    Implementation  UMETA(DisplayName = "Implementation"),
    Integration     UMETA(DisplayName = "Integration"),
    Testing         UMETA(DisplayName = "Testing"),
    Delivery        UMETA(DisplayName = "Delivery")
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
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<int32> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString ExpectedOutput;

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
        ExpectedOutput = TEXT("");
        StartTime = FDateTime::Now();
        DeadlineTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 UE5ActorsSpawned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bMilestone1Complete;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        CompilationErrors = 0;
        UE5ActorsSpawned = 0;
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
        bMilestone1Complete = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDirectorCoordinator : public UActorComponent
{
    GENERATED_BODY()

public:
    UDirectorCoordinator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void InitializeAgentPipeline();

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void AssignTaskToAgent(int32 AgentID, const FString& TaskDescription, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void UpdateAgentStatus(int32 AgentID, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    bool CanAgentStart(int32 AgentID);

    // Production Tracking
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    // Milestone Management
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void CheckMilestone1Progress();

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    bool IsMilestone1Complete() const;

    // Emergency Protocols
    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void TriggerEmergencyCoordination();

    UFUNCTION(BlueprintCallable, Category = "Director Coordination")
    void ResetProductionPipeline();

    // Debug and Monitoring
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Director Coordination")
    void DebugPrintAgentStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Director Coordination")
    void ForceAgentCompletion(int32 AgentID);

protected:
    // Agent Pipeline
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Pipeline")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Pipeline")
    int32 CurrentActiveAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Pipeline")
    EDir_MilestonePhase CurrentPhase;

    // Production State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    bool bEmergencyMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production State")
    float ProductionStartTime;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AgentTimeoutMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoAdvanceAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bStrictDependencyChecking;

private:
    void SetupAgentDependencies();
    void ValidateAgentOutputs();
    void CalculateProductionVelocity();
    bool CheckSystemHealth();
    void LogProductionState();
};

#include "DirectorCoordinator.generated.h"