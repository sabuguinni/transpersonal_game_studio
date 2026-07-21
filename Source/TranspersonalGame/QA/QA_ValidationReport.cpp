#include "QA_ValidationReport.h"

UQA_ValidationReport::UQA_ValidationReport()
{
    ReportTitle = TEXT("Transpersonal Game QA Validation Report");
    ReportTimestamp = FDateTime::Now();
    TotalActorsInLevel = 0;
    CriticalSystemsOnline = 0;
    OverallValidationTime = 0.0f;
    bReadyForIntegration = false;
    IntegrationNotes = TEXT("Validation in progress...");
}

void UQA_ValidationReport::AddSystemReport(const FString& SystemName, EQA_SystemStatus Status, const FString& Message)
{
    FQA_SystemReport NewReport;
    NewReport.SystemName = SystemName;
    NewReport.Status = Status;
    NewReport.StatusMessage = Message;
    NewReport.ValidationTime = FPlatformTime::Seconds();
    
    // Set test counts based on status
    switch (Status)
    {
        case EQA_SystemStatus::Operational:
            NewReport.TestsPassed = 1;
            NewReport.TestsFailed = 0;
            break;
        case EQA_SystemStatus::Warning:
            NewReport.TestsPassed = 1;
            NewReport.TestsFailed = 0;
            break;
        case EQA_SystemStatus::Critical:
        case EQA_SystemStatus::Offline:
            NewReport.TestsPassed = 0;
            NewReport.TestsFailed = 1;
            break;
        default:
            NewReport.TestsPassed = 0;
            NewReport.TestsFailed = 0;
            break;
    }
    
    SystemReports.Add(NewReport);
    
    // Update critical systems count
    if (Status == EQA_SystemStatus::Operational)
    {
        CriticalSystemsOnline++;
    }
}

float UQA_ValidationReport::GetSuccessRate() const
{
    if (SystemReports.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 SuccessfulSystems = 0;
    for (const FQA_SystemReport& Report : SystemReports)
    {
        if (Report.Status == EQA_SystemStatus::Operational || Report.Status == EQA_SystemStatus::Warning)
        {
            SuccessfulSystems++;
        }
    }
    
    return (float)SuccessfulSystems / SystemReports.Num() * 100.0f;
}

int32 UQA_ValidationReport::GetCriticalIssueCount() const
{
    int32 CriticalCount = 0;
    for (const FQA_SystemReport& Report : SystemReports)
    {
        if (Report.Status == EQA_SystemStatus::Critical || Report.Status == EQA_SystemStatus::Offline)
        {
            CriticalCount++;
        }
    }
    return CriticalCount;
}

FString UQA_ValidationReport::GenerateReportSummary() const
{
    FString Summary;
    Summary += FString::Printf(TEXT("=== QA VALIDATION SUMMARY ===\n"));
    Summary += FString::Printf(TEXT("Report: %s\n"), *ReportTitle);
    Summary += FString::Printf(TEXT("Timestamp: %s\n"), *ReportTimestamp.ToString());
    Summary += FString::Printf(TEXT("Systems Tested: %d\n"), SystemReports.Num());
    Summary += FString::Printf(TEXT("Success Rate: %.1f%%\n"), GetSuccessRate());
    Summary += FString::Printf(TEXT("Critical Issues: %d\n"), GetCriticalIssueCount());
    Summary += FString::Printf(TEXT("Critical Systems Online: %d\n"), CriticalSystemsOnline);
    Summary += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorsInLevel);
    Summary += FString::Printf(TEXT("Validation Time: %.2f seconds\n"), OverallValidationTime);
    Summary += FString::Printf(TEXT("Ready for Integration: %s\n"), bReadyForIntegration ? TEXT("YES") : TEXT("NO"));
    
    if (!IntegrationNotes.IsEmpty())
    {
        Summary += FString::Printf(TEXT("Integration Notes: %s\n"), *IntegrationNotes);
    }
    
    Summary += TEXT("\n=== SYSTEM STATUS DETAILS ===\n");
    for (const FQA_SystemReport& Report : SystemReports)
    {
        FString StatusText;
        switch (Report.Status)
        {
            case EQA_SystemStatus::Operational:
                StatusText = TEXT("OPERATIONAL");
                break;
            case EQA_SystemStatus::Warning:
                StatusText = TEXT("WARNING");
                break;
            case EQA_SystemStatus::Critical:
                StatusText = TEXT("CRITICAL");
                break;
            case EQA_SystemStatus::Offline:
                StatusText = TEXT("OFFLINE");
                break;
            default:
                StatusText = TEXT("NOT TESTED");
                break;
        }
        
        Summary += FString::Printf(TEXT("%s: %s - %s\n"), 
                                   *Report.SystemName, 
                                   *StatusText, 
                                   *Report.StatusMessage);
    }
    
    Summary += TEXT("=== END REPORT ===\n");
    return Summary;
}