#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirectorSystem.generated.h"

/**
 * Production metrics for tracking agent performance and deliverables
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CycleExecutionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Production")
    bool bMinPlayableMapReady;

    FDir_ProductionMetrics()
    {
        TotalActorsInLevel = 0;
        CompilationErrors = 0;
        ActiveAgentCount = 0;
        CycleExecutionTime = 0.0f;
        bMinPlayableMapReady = false;
    }
};

/**
 * Agent task priority levels for production coordination
 */
UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical = 0    UMETA(DisplayName = "Critical"),
    High = 1        UMETA(DisplayName = "High"),
    Normal = 2      UMETA(DisplayName = "Normal"),
    Low = 3         UMETA(DisplayName = "Low"),
    Deferred = 4    UMETA(DisplayName = "Deferred")
};

/**
 * Agent task definition for coordinated development
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    FString TaskDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    EDir_AgentPriority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    float EstimatedDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Task")
    bool bIsBlocking;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_AgentPriority::Normal;
        EstimatedDuration = 0.0f;
        bIsBlocking = false;
    }
};

/**
 * Studio Director System - Coordinates all agent activities and production pipeline
 * Manages the 19-agent chain execution, tracks deliverables, and ensures quality
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

    /**
     * Initialize production cycle and prepare agent coordination
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle();

    /**
     * Queue task for specific agent with priority and requirements
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void QueueAgentTask(const FString& AgentName, const FString& TaskDescription, 
                       EDir_AgentPriority Priority, const TArray<FString>& Deliverables);

    /**
     * Get current production metrics for dashboard display
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetProductionMetrics() const;

    /**
     * Mark agent task as completed and update metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteAgentTask(const FString& AgentName, bool bSuccess);

    /**
     * Check if MinPlayableMap is ready for agent modifications
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMinPlayableMapReady() const;

    /**
     * Validate compilation status and actor count
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateProjectState() const;

    /**
     * Get next agent in the production chain
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetNextAgent(const FString& CurrentAgent) const;

    /**
     * Emergency stop production if critical errors detected
     */
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStopProduction(const FString& Reason);

protected:
    /**
     * Current production metrics
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FDir_ProductionMetrics CurrentMetrics;

    /**
     * Queue of pending agent tasks
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> PendingTasks;

    /**
     * Completed agent tasks this cycle
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FDir_AgentTask> CompletedTasks;

    /**
     * Production chain agent names in execution order
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    TArray<FString> AgentChain;

    /**
     * Current production cycle ID
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    FString CurrentCycleID;

    /**
     * Production start timestamp
     */
    UPROPERTY(BlueprintReadOnly, Category = "Production")
    float CycleStartTime;

private:
    /**
     * Update production metrics from current world state
     */
    void UpdateProductionMetrics();

    /**
     * Initialize agent execution chain
     */
    void InitializeAgentChain();

    /**
     * Log production event for tracking
     */
    void LogProductionEvent(const FString& Event, const FString& Details);
};