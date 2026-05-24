#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_CycleIntegrationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 ToolCallsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 FilesWritten;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    float ExecutionTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    bool bCompilationSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    bool bValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle Metrics")
    TArray<FString> WarningsGenerated;

    FBuild_CycleMetrics()
    {
        CycleID = TEXT("");
        AgentNumber = 0;
        ToolCallsExecuted = 0;
        FilesWritten = 0;
        UE5CommandsExecuted = 0;
        ExecutionTimeSeconds = 0.0f;
        bCompilationSuccessful = false;
        bValidationPassed = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    bool bCompilationClean;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    int32 ClassesLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    int32 ActorsSpawned;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    float PerformanceImpact;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System Status")
    TArray<FString> IntegrationIssues;

    FBuild_SystemIntegrationStatus()
    {
        SystemName = TEXT("");
        bIsActive = false;
        bCompilationClean = false;
        ClassesLoaded = 0;
        ActorsSpawned = 0;
        PerformanceImpact = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FDateTime ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_CycleMetrics> AgentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FBuild_SystemIntegrationStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 TotalFilesInProject;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bOverallIntegrationHealthy;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    float OverallPerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> CriticalBlockers;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> RecommendedActions;

    FBuild_CycleIntegrationReport()
    {
        CycleID = TEXT("");
        ReportTimestamp = FDateTime::Now();
        TotalFilesInProject = 0;
        TotalActorsInLevel = 0;
        bOverallIntegrationHealthy = false;
        OverallPerformanceScore = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_CycleIntegrationReportSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void GenerateCycleReport(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    FBuild_CycleIntegrationReport GetLatestReport() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool IsIntegrationHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    TArray<FString> GetCriticalIssues() const;

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void LogIntegrationMetrics();

protected:
    UPROPERTY()
    FBuild_CycleIntegrationReport CurrentReport;

    UPROPERTY()
    TArray<FBuild_CycleIntegrationReport> ReportHistory;

    void CollectAgentMetrics();
    void ValidateSystemStatuses();
    void CalculatePerformanceMetrics();
    void IdentifyCriticalBlockers();
    void GenerateRecommendations();
};