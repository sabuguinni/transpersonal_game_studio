#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Studio Director System - Coordinates agent outputs and manages production pipeline
 * Ensures all agent implementations work together cohesively
 */

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Deferred    UMETA(DisplayName = "Deferred")
};

UENUM(BlueprintType)
enum class EDir_SystemStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Testing     UMETA(DisplayName = "Testing"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
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
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedHours;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_SystemStatus::NotStarted;
        ExpectedDeliverable = TEXT("");
        EstimatedHours = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CompletedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float TerrainCoverage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bPlayablePrototypeReady;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        CompilationErrors = 0;
        CompletedSystems = 0;
        TerrainCoverage = 0.0f;
        bPlayablePrototypeReady = false;
    }
};

/**
 * Studio Director System - Game Instance Subsystem
 * Manages production coordination and agent task tracking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddAgentTask(const FDir_AgentTask& NewTask);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateTaskStatus(const FString& AgentName, EDir_SystemStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksByPriority(EDir_AgentPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetTaskByAgent(const FString& AgentName);

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPlayablePrototypeReady();

    // Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CoordinateAgentHandoff(const FString& FromAgent, const FString& ToAgent);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateSystemIntegration();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void DebugPrintAllTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor = true)
    void RunProductionHealthCheck();

protected:
    // Task tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tasks")
    TArray<FDir_AgentTask> AgentTasks;

    // Production metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
    FDir_ProductionMetrics CurrentMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MetricsUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxConcurrentTasks;

private:
    // Internal tracking
    FTimerHandle MetricsUpdateTimer;
    
    // Helper functions
    void InitializeDefaultTasks();
    void CalculateTerrainCoverage();
    void CheckCompilationStatus();
    void ValidateActorCounts();
};