#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QA_ValidationReport.generated.h"

UENUM(BlueprintType)
enum class EQA_SystemStatus : uint8
{
    NotTested   UMETA(DisplayName = "Not Tested"),
    Operational UMETA(DisplayName = "Operational"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Offline     UMETA(DisplayName = "Offline")
};

USTRUCT(BlueprintType)
struct FQA_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    EQA_SystemStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString StatusMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 TestsPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 TestsFailed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    float ValidationTime;

    FQA_SystemReport()
    {
        SystemName = TEXT("");
        Status = EQA_SystemStatus::NotTested;
        StatusMessage = TEXT("");
        TestsPassed = 0;
        TestsFailed = 0;
        ValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQA_ValidationReport : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString ReportTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FDateTime ReportTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FQA_SystemReport> SystemReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 TotalActorsInLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 CriticalSystemsOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    float OverallValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bReadyForIntegration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString IntegrationNotes;

public:
    UQA_ValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void AddSystemReport(const FString& SystemName, EQA_SystemStatus Status, const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float GetSuccessRate() const;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    int32 GetCriticalIssueCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GenerateReportSummary() const;
};