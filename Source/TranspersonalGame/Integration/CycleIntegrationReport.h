#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CycleIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EIntegrationCycleStatus : uint8
{
    NotStarted,
    InProgress,
    Completed,
    Failed,
    RequiresAttention
};

USTRUCT(BlueprintType)
struct FAgentCycleReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 AgentID;

    UPROPERTY(BlueprintReadOnly)
    FString AgentName;

    UPROPERTY(BlueprintReadOnly)
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly)
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly)
    bool bCompilationSuccess;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> ErrorMessages;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTimeSeconds;

    FAgentCycleReport()
        : AgentID(0)
        , FilesCreated(0)
        , UE5CommandsExecuted(0)
        , bCompilationSuccess(false)
        , ExecutionTimeSeconds(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCycleIntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 TotalAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 CompletedAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 FailedAgents;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalFilesCreated;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalUE5Commands;

    UPROPERTY(BlueprintReadOnly)
    int32 CompilationErrors;

    UPROPERTY(BlueprintReadOnly)
    float TotalExecutionTime;

    UPROPERTY(BlueprintReadOnly)
    float AverageAgentTime;

    FCycleIntegrationMetrics()
        : TotalAgents(0)
        , CompletedAgents(0)
        , FailedAgents(0)
        , TotalFilesCreated(0)
        , TotalUE5Commands(0)
        , CompilationErrors(0)
        , TotalExecutionTime(0.0f)
        , AverageAgentTime(0.0f)
    {}
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCycleIntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    FDateTime CycleStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    FDateTime CycleEndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    EIntegrationCycleStatus CycleStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    TArray<FAgentCycleReport> AgentReports;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    FCycleIntegrationMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Report")
    TArray<FString> Recommendations;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void InitializeCycle(const FString& InCycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void AddAgentReport(const FAgentCycleReport& AgentReport);

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    void FinalizeCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    FString GenerateReportSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    bool HasCriticalIssues() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    float GetCycleSuccessRate() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Report")
    TArray<FString> GetTopIssues() const;

private:
    void CalculateMetrics();
    void AnalyzeCriticalIssues();
    void GenerateRecommendations();
};