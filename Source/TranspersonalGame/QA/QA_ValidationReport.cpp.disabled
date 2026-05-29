#include "QA_ValidationReport.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UQA_ValidationReport::UQA_ValidationReport()
{
    CurrentCycleID = TEXT("UNKNOWN_CYCLE");
}

void UQA_ValidationReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QA Validation Report System Initialized"));
    
    // Set current cycle ID from environment or default
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260518_003");
    
    // Initialize system health with default values
    SystemHealth = FQA_SystemHealth();
}

void UQA_ValidationReport::Deinitialize()
{
    // Save final report before shutdown
    GenerateReport();
    
    Super::Deinitialize();
}

void UQA_ValidationReport::AddTestResult(const FString& TestName, EQA_ValidationStatus Status, const FString& Message, float ExecutionTime)
{
    FQA_TestResult NewResult;
    NewResult.TestName = TestName;
    NewResult.Status = Status;
    NewResult.Message = Message;
    NewResult.ExecutionTime = ExecutionTime;
    
    TestResults.Add(NewResult);
    
    // Log the result immediately
    LogTestResult(NewResult);
}

void UQA_ValidationReport::UpdateSystemHealth(int32 ActorCount, float MemoryUsage, int32 ClassCount, int32 AssetCount, bool bHealthy)
{
    SystemHealth.ActorCount = ActorCount;
    SystemHealth.MemoryUsagePercent = MemoryUsage;
    SystemHealth.LoadedClassCount = ClassCount;
    SystemHealth.AvailableAssetCount = AssetCount;
    SystemHealth.bBridgeHealthy = bHealthy;
    
    UE_LOG(LogTemp, Log, TEXT("QA System Health Updated: Actors=%d, Memory=%.1f%%, Classes=%d, Assets=%d, Healthy=%s"),
        ActorCount, MemoryUsage, ClassCount, AssetCount, bHealthy ? TEXT("Yes") : TEXT("No"));
}

void UQA_ValidationReport::GenerateReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VALIDATION REPORT - %s ==="), *CurrentCycleID);
    
    // Count results by status
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    int32 CriticalCount = 0;
    
    for (const FQA_TestResult& Result : TestResults)
    {
        switch (Result.Status)
        {
            case EQA_ValidationStatus::Pass:
                PassCount++;
                break;
            case EQA_ValidationStatus::Warning:
                WarningCount++;
                break;
            case EQA_ValidationStatus::Fail:
                FailCount++;
                break;
            case EQA_ValidationStatus::Critical:
                CriticalCount++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Test Results Summary:"));
    UE_LOG(LogTemp, Warning, TEXT("  PASS: %d"), PassCount);
    UE_LOG(LogTemp, Warning, TEXT("  WARN: %d"), WarningCount);
    UE_LOG(LogTemp, Warning, TEXT("  FAIL: %d"), FailCount);
    UE_LOG(LogTemp, Warning, TEXT("  CRITICAL: %d"), CriticalCount);
    
    UE_LOG(LogTemp, Warning, TEXT("System Health:"));
    UE_LOG(LogTemp, Warning, TEXT("  Actors: %d"), SystemHealth.ActorCount);
    UE_LOG(LogTemp, Warning, TEXT("  Memory: %.1f%%"), SystemHealth.MemoryUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("  Classes: %d"), SystemHealth.LoadedClassCount);
    UE_LOG(LogTemp, Warning, TEXT("  Assets: %d"), SystemHealth.AvailableAssetCount);
    UE_LOG(LogTemp, Warning, TEXT("  Bridge: %s"), SystemHealth.bBridgeHealthy ? TEXT("HEALTHY") : TEXT("DEGRADED"));
    
    // Overall status
    bool bOverallPass = (CriticalCount == 0) && (FailCount < 3) && SystemHealth.bBridgeHealthy;
    UE_LOG(LogTemp, Warning, TEXT("OVERALL STATUS: %s"), bOverallPass ? TEXT("PASS") : TEXT("NEEDS ATTENTION"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== END QA REPORT ==="));
    
    // Save to file
    SaveReportToFile();
}

void UQA_ValidationReport::ClearResults()
{
    TestResults.Empty();
    SystemHealth = FQA_SystemHealth();
    
    UE_LOG(LogTemp, Log, TEXT("QA Validation results cleared"));
}

int32 UQA_ValidationReport::GetPassCount() const
{
    int32 Count = 0;
    for (const FQA_TestResult& Result : TestResults)
    {
        if (Result.Status == EQA_ValidationStatus::Pass)
        {
            Count++;
        }
    }
    return Count;
}

int32 UQA_ValidationReport::GetFailCount() const
{
    int32 Count = 0;
    for (const FQA_TestResult& Result : TestResults)
    {
        if (Result.Status == EQA_ValidationStatus::Fail || Result.Status == EQA_ValidationStatus::Critical)
        {
            Count++;
        }
    }
    return Count;
}

void UQA_ValidationReport::LogTestResult(const FQA_TestResult& Result)
{
    FString StatusString;
    switch (Result.Status)
    {
        case EQA_ValidationStatus::Pass:
            StatusString = TEXT("PASS");
            break;
        case EQA_ValidationStatus::Warning:
            StatusString = TEXT("WARN");
            break;
        case EQA_ValidationStatus::Fail:
            StatusString = TEXT("FAIL");
            break;
        case EQA_ValidationStatus::Critical:
            StatusString = TEXT("CRITICAL");
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("QA [%s] %s: %s (%.2fs)"), 
        *StatusString, *Result.TestName, *Result.Message, Result.ExecutionTime);
}

void UQA_ValidationReport::SaveReportToFile()
{
    FString ReportContent;
    ReportContent += FString::Printf(TEXT("QA Validation Report - %s\n"), *CurrentCycleID);
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    // Add test results
    ReportContent += TEXT("Test Results:\n");
    for (const FQA_TestResult& Result : TestResults)
    {
        FString StatusString;
        switch (Result.Status)
        {
            case EQA_ValidationStatus::Pass: StatusString = TEXT("PASS"); break;
            case EQA_ValidationStatus::Warning: StatusString = TEXT("WARN"); break;
            case EQA_ValidationStatus::Fail: StatusString = TEXT("FAIL"); break;
            case EQA_ValidationStatus::Critical: StatusString = TEXT("CRITICAL"); break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s: %s (%.2fs)\n"), 
            *StatusString, *Result.TestName, *Result.Message, Result.ExecutionTime);
    }
    
    // Add system health
    ReportContent += TEXT("\nSystem Health:\n");
    ReportContent += FString::Printf(TEXT("Actors: %d\n"), SystemHealth.ActorCount);
    ReportContent += FString::Printf(TEXT("Memory: %.1f%%\n"), SystemHealth.MemoryUsagePercent);
    ReportContent += FString::Printf(TEXT("Classes: %d\n"), SystemHealth.LoadedClassCount);
    ReportContent += FString::Printf(TEXT("Assets: %d\n"), SystemHealth.AvailableAssetCount);
    ReportContent += FString::Printf(TEXT("Bridge: %s\n"), SystemHealth.bBridgeHealthy ? TEXT("HEALTHY") : TEXT("DEGRADED"));
    
    // Save to temp file
    FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("QA"), FString::Printf(TEXT("ValidationReport_%s.txt"), *CurrentCycleID));
    
    // Ensure directory exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));
    
    // Write file
    if (FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("QA Report saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save QA Report to: %s"), *FilePath);
    }
}