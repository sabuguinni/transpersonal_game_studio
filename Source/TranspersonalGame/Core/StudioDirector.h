#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "StudioDirector.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_AgentTaskAssignment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    FString Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    TArray<FString> RequiredDeliverables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    TArray<FString> RequiredFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    TArray<FString> RequiredUE5Commands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    bool bTaskCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    FDateTime TaskAssignedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Assignment")
    FDateTime TaskCompletedTime;

    FDir_AgentTaskAssignment()
    {
        AgentName = TEXT("");
        Priority = TEXT("MEDIUM");
        bTaskCompleted = false;
        TaskAssignedTime = FDateTime::Now();
        TaskCompletedTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_ProductionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalCppFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 TotalHeaderFiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 OrphanedHeaders;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 CompilationErrors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    int32 ActorsInMinPlayableMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bProjectCompiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bMinPlayableMapLoads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    bool bPlayerCanMove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Metrics")
    FDateTime LastAssessmentTime;

    FDir_ProductionMetrics()
    {
        TotalCppFiles = 0;
        TotalHeaderFiles = 0;
        OrphanedHeaders = 0;
        CompilationErrors = 0;
        ActorsInMinPlayableMap = 0;
        bProjectCompiles = false;
        bMinPlayableMapLoads = false;
        bPlayerCanMove = false;
        LastAssessmentTime = FDateTime::Now();
    }
};

/**
 * Studio Director - Agent #01
 * Coordinates all 19 agents in the production chain
 * Translates creative vision into concrete technical tasks
 * Monitors production metrics and ensures milestone delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirector : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirector();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Task Assignment System
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(const FString& AgentName, const FDir_AgentTaskAssignment& TaskAssignment);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void MarkTaskCompleted(const FString& AgentName, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTaskAssignment> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTaskAssignment> GetCompletedTasks() const;

    // Production Metrics
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssessProductionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_ProductionMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogProductionStatus() const;

    // Milestone Tracking
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsMilestone1Complete() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetMilestone1Progress() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetMilestone1Requirements() const;

    // Agent Chain Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentChain();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void DispatchNextAgent();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetCurrentActiveAgent() const { return CurrentActiveAgent; }

    // Crisis Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleCompilationFailure();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void HandleAgentTimeout(const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyReset();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TMap<FString, FDir_AgentTaskAssignment> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTaskAssignment> CompletedTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDir_ProductionMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FString> AgentChain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentAgentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FString CurrentActiveAgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    int32 CurrentCycleNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    FDateTime CycleStartTime;

private:
    void InitializeAgentChainArray();
    void CreateDefaultTaskAssignments();
    void ValidateProjectStructure();
    bool CheckMinPlayableMapState();
    void GenerateProductionReport();
};