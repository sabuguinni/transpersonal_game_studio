#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "StudioDirector.generated.h"

/**
 * Studio Director System - Central coordination for all game systems
 * Manages agent task distribution, production priorities, and quality assurance
 * Ensures all agents work toward unified game vision
 */

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task") 
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    EDir_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    float EstimatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bIsBlocking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FString ExpectedDeliverable;

    FDir_AgentTask()
    {
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Priority = EDir_Priority::Medium;
        EstimatedTime = 60.0f;
        bIsBlocking = false;
        ExpectedDeliverable = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_ProductionStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 TotalActorsInMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 HeadersWithoutCpp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bCanPlaytest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FString BlockingIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    float OverallProgress;

    FDir_ProductionStatus()
    {
        TotalActorsInMap = 0;
        CompilationErrors = 0;
        HeadersWithoutCpp = 0;
        bCanPlaytest = false;
        BlockingIssues = TEXT("");
        OverallProgress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirector : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirector();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FString& TaskDescription, EDir_Priority Priority, bool bIsBlocking = false);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetTasksForAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteTask(const FString& AgentName, const FString& TaskDescription);

    // Production Monitoring
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionStatus GetProductionStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool CanAgentProceed(const FString& AgentName) const;

    // Quality Assurance
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportCompilationError(const FString& ErrorMessage, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateGameplayReadiness();

    // Coordination
    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void CoordinateAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director", CallInEditor)
    void GenerateProductionReport();

protected:
    // Task tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tasks")
    TArray<FDir_AgentTask> ActiveTasks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tasks")
    TArray<FDir_AgentTask> CompletedTasks;

    // Production metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    FDir_ProductionStatus CurrentStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FString> BlockingIssues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Production")
    TArray<FString> CompilationErrors;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutoCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float CoordinationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxConcurrentTasks;

private:
    // Internal methods
    void ScanForCompilationIssues();
    void AssessGameplayReadiness();
    void PrioritizeTasks();
    void LogProductionMetrics() const;

    // Timer handles
    FTimerHandle CoordinationTimerHandle;
};