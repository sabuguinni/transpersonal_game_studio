#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Build_CycleCompletionReport.generated.h"

/**
 * Cycle Completion Report - Integration Agent #19
 * Final report for production cycle completion
 * Documents all systems, validation results, and deployment readiness
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_AgentDeliverable
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    int32 AgentNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    TArray<FString> FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    TArray<FString> SystemsImplemented;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    int32 UE5CommandsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    bool bCompilationSuccessful = false;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    bool bValidationPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    FString CompletionStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Deliverable")
    float ExecutionTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString CycleStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FString CycleEndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float TotalExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalAgentsExecuted = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalFilesCreated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalUE5Commands = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 SuccessfulAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FailedAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float OverallSuccessRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bCycleCompletedSuccessfully = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bHasImplementation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bPassedValidation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 ComponentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> KnownIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float HealthScore = 0.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CycleCompletionReport : public UDataAsset
{
    GENERATED_BODY()

public:
    UBuild_CycleCompletionReport();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "Report Generation")
    void GenerateCompletionReport(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Report Generation")
    void AddAgentDeliverable(const FBuild_AgentDeliverable& Deliverable);

    UFUNCTION(BlueprintCallable, Category = "Report Generation")
    void AddSystemHealthReport(const FBuild_SystemHealthReport& HealthReport);

    UFUNCTION(BlueprintCallable, Category = "Report Generation")
    void CalculateCycleMetrics();

    UFUNCTION(BlueprintCallable, Category = "Report Generation")
    void FinalizeReport();

    // Report access
    UFUNCTION(BlueprintCallable, Category = "Report Access")
    FBuild_CycleMetrics GetCycleMetrics() const { return CycleMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Report Access")
    TArray<FBuild_AgentDeliverable> GetAgentDeliverables() const { return AgentDeliverables; }

    UFUNCTION(BlueprintCallable, Category = "Report Access")
    TArray<FBuild_SystemHealthReport> GetSystemHealthReports() const { return SystemHealthReports; }

    UFUNCTION(BlueprintCallable, Category = "Report Access")
    bool IsCycleSuccessful() const { return CycleMetrics.bCycleCompletedSuccessfully; }

    UFUNCTION(BlueprintCallable, Category = "Report Access")
    float GetOverallHealthScore() const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool CheckDeploymentReadiness();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetCriticalIssues();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetRecommendations();

    // Export functions
    UFUNCTION(BlueprintCallable, Category = "Export")
    FString ExportToMarkdown();

    UFUNCTION(BlueprintCallable, Category = "Export")
    FString ExportToJSON();

    UFUNCTION(BlueprintCallable, Category = "Export")
    void SaveReportToFile(const FString& FilePath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    FBuild_CycleMetrics CycleMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    TArray<FBuild_AgentDeliverable> AgentDeliverables;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    TArray<FBuild_SystemHealthReport> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    TArray<FString> Recommendations;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    bool bReportFinalized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Report Data")
    FString ReportGenerationTime;

private:
    void AnalyzeCriticalIssues();
    void GenerateRecommendations();
    float CalculateSystemHealthScore(const FBuild_SystemHealthReport& HealthReport);
};