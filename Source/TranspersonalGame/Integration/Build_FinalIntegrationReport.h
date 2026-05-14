#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Build_FinalIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    RequiresReview  UMETA(DisplayName = "Requires Review")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString LastValidationResult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> DependentSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FString> ValidationErrors;

    FBuild_SystemIntegrationData()
    {
        SystemName = TEXT("Unknown");
        Status = EBuild_IntegrationStatus::Unknown;
        CompletionPercentage = 0.0f;
        LastValidationResult = TEXT("Not Validated");
        LastUpdateTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalReportData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FString BuildVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FDateTime BuildTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 TotalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 CompletedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 FailedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    float OverallCompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FBuild_SystemIntegrationData> SystemReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> CriticalIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FString> Recommendations;

    FBuild_FinalReportData()
    {
        BuildVersion = TEXT("1.0.0");
        BuildTimestamp = FDateTime::Now();
        TotalSystems = 0;
        CompletedSystems = 0;
        FailedSystems = 0;
        OverallCompletionPercentage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalIntegrationReport : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalIntegrationReport();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ReportVisualizationMesh;

public:
    virtual void Tick(float DeltaTime) override;

    // Final Integration Report Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    FBuild_FinalReportData FinalReportData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    bool bAutoGenerateReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    float ReportGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    FString ReportOutputPath;

    // Integration Report Functions
    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void GenerateFinalIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void AddSystemReport(const FBuild_SystemIntegrationData& SystemData);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void UpdateSystemStatus(const FString& SystemName, EBuild_IntegrationStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    FBuild_SystemIntegrationData GetSystemReport(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    TArray<FString> GetFailedSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    TArray<FString> GetCompletedSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    float CalculateOverallProgress();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ExportReportToFile();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ImportReportFromFile();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ResetAllReports();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void PerformSystemHealthCheck();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void GenerateRecommendations();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    bool ValidateSystemDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void ScheduleAutomaticReporting();

    UFUNCTION(BlueprintCallable, Category = "Integration Report")
    void SendReportToStakeholders();

private:
    float LastReportTime;
    bool bReportingActive;

    void InitializeDefaultSystems();
    void UpdateReportVisualization();
    void LogReportData();
    void ValidateReportIntegrity();
    void ProcessSystemDependencies();
    void AnalyzePerformanceMetrics();
    void GenerateCriticalIssuesList();
    void CreateRecommendationsList();
    void SaveReportToDatabase();
    void NotifyStakeholders();
};