#include "QAReportGenerator.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogQAReportGenerator);

FQAReportGenerator::FQAReportGenerator()
    : bIsGeneratingReport(false)
    , CurrentReportSession(nullptr)
{
    UE_LOG(LogQAReportGenerator, Log, TEXT("QA Report Generator initialized"));
}

FQAReportGenerator::~FQAReportGenerator()
{
    if (CurrentReportSession)
    {
        EndReportSession();
    }
}

void FQAReportGenerator::StartReportSession(const FString& SessionName, const FString& BuildVersion)
{
    if (bIsGeneratingReport)
    {
        UE_LOG(LogQAReportGenerator, Warning, TEXT("Report session already in progress. Ending current session."));
        EndReportSession();
    }

    CurrentReportSession = MakeShared<FQAReportSession>();
    CurrentReportSession->SessionName = SessionName;
    CurrentReportSession->BuildVersion = BuildVersion;
    CurrentReportSession->StartTime = FDateTime::Now();
    CurrentReportSession->Platform = FPlatformProperties::PlatformName();
    CurrentReportSession->EngineVersion = FEngineVersion::Current().ToString();
    
    bIsGeneratingReport = true;
    
    UE_LOG(LogQAReportGenerator, Log, TEXT("Started QA report session: %s (Build: %s)"), *SessionName, *BuildVersion);
}

void FQAReportGenerator::EndReportSession()
{
    if (!bIsGeneratingReport || !CurrentReportSession.IsValid())
    {
        UE_LOG(LogQAReportGenerator, Warning, TEXT("No active report session to end"));
        return;
    }

    CurrentReportSession->EndTime = FDateTime::Now();
    CurrentReportSession->Duration = CurrentReportSession->EndTime - CurrentReportSession->StartTime;
    
    // Generate final report
    GenerateHTMLReport();
    GenerateJSONReport();
    GenerateCSVReport();
    
    UE_LOG(LogQAReportGenerator, Log, TEXT("Ended QA report session: %s"), *CurrentReportSession->SessionName);
    
    bIsGeneratingReport = false;
    CurrentReportSession.Reset();
}

void FQAReportGenerator::AddTestResult(const FQATestResult& TestResult)
{
    if (!bIsGeneratingReport || !CurrentReportSession.IsValid())
    {
        UE_LOG(LogQAReportGenerator, Warning, TEXT("No active report session for test result: %s"), *TestResult.TestName);
        return;
    }

    CurrentReportSession->TestResults.Add(TestResult);
    
    // Update session statistics
    switch (TestResult.Result)
    {
        case EQATestResult::Passed:
            CurrentReportSession->PassedTests++;
            break;
        case EQATestResult::Failed:
            CurrentReportSession->FailedTests++;
            break;
        case EQATestResult::Skipped:
            CurrentReportSession->SkippedTests++;
            break;
        case EQATestResult::Warning:
            CurrentReportSession->WarningTests++;
            break;
    }
    
    CurrentReportSession->TotalTests = CurrentReportSession->TestResults.Num();
    
    UE_LOG(LogQAReportGenerator, Verbose, TEXT("Added test result: %s - %s"), 
           *TestResult.TestName, 
           *UEnum::GetValueAsString(TestResult.Result));
}

void FQAReportGenerator::AddPerformanceMetric(const FQAPerformanceMetric& Metric)
{
    if (!bIsGeneratingReport || !CurrentReportSession.IsValid())
    {
        UE_LOG(LogQAReportGenerator, Warning, TEXT("No active report session for performance metric: %s"), *Metric.MetricName);
        return;
    }

    CurrentReportSession->PerformanceMetrics.Add(Metric);
    
    UE_LOG(LogQAReportGenerator, Verbose, TEXT("Added performance metric: %s = %.2f"), 
           *Metric.MetricName, 
           Metric.Value);
}

void FQAReportGenerator::AddBugReport(const FQABugReport& BugReport)
{
    if (!bIsGeneratingReport || !CurrentReportSession.IsValid())
    {
        UE_LOG(LogQAReportGenerator, Warning, TEXT("No active report session for bug report: %s"), *BugReport.Title);
        return;
    }

    CurrentReportSession->BugReports.Add(BugReport);
    
    UE_LOG(LogQAReportGenerator, Log, TEXT("Added bug report: %s (Severity: %s)"), 
           *BugReport.Title, 
           *UEnum::GetValueAsString(BugReport.Severity));
}

void FQAReportGenerator::GenerateHTMLReport()
{
    if (!CurrentReportSession.IsValid())
    {
        return;
    }

    FString HTMLContent = GenerateHTMLContent();
    FString ReportPath = GetReportPath(TEXT("html"));
    
    if (FFileHelper::SaveStringToFile(HTMLContent, *ReportPath))
    {
        UE_LOG(LogQAReportGenerator, Log, TEXT("HTML report generated: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogQAReportGenerator, Error, TEXT("Failed to save HTML report: %s"), *ReportPath);
    }
}

void FQAReportGenerator::GenerateJSONReport()
{
    if (!CurrentReportSession.IsValid())
    {
        return;
    }

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    // Session information
    JsonObject->SetStringField(TEXT("sessionName"), CurrentReportSession->SessionName);
    JsonObject->SetStringField(TEXT("buildVersion"), CurrentReportSession->BuildVersion);
    JsonObject->SetStringField(TEXT("platform"), CurrentReportSession->Platform);
    JsonObject->SetStringField(TEXT("engineVersion"), CurrentReportSession->EngineVersion);
    JsonObject->SetStringField(TEXT("startTime"), CurrentReportSession->StartTime.ToString());
    JsonObject->SetStringField(TEXT("endTime"), CurrentReportSession->EndTime.ToString());
    JsonObject->SetNumberField(TEXT("durationSeconds"), CurrentReportSession->Duration.GetTotalSeconds());
    
    // Test statistics
    TSharedPtr<FJsonObject> StatsObject = MakeShareable(new FJsonObject);
    StatsObject->SetNumberField(TEXT("totalTests"), CurrentReportSession->TotalTests);
    StatsObject->SetNumberField(TEXT("passedTests"), CurrentReportSession->PassedTests);
    StatsObject->SetNumberField(TEXT("failedTests"), CurrentReportSession->FailedTests);
    StatsObject->SetNumberField(TEXT("skippedTests"), CurrentReportSession->SkippedTests);
    StatsObject->SetNumberField(TEXT("warningTests"), CurrentReportSession->WarningTests);
    JsonObject->SetObjectField(TEXT("statistics"), StatsObject);
    
    // Test results
    TArray<TSharedPtr<FJsonValue>> TestResultsArray;
    for (const FQATestResult& TestResult : CurrentReportSession->TestResults)
    {
        TSharedPtr<FJsonObject> TestObject = MakeShareable(new FJsonObject);
        TestObject->SetStringField(TEXT("testName"), TestResult.TestName);
        TestObject->SetStringField(TEXT("category"), TestResult.Category);
        TestObject->SetStringField(TEXT("result"), UEnum::GetValueAsString(TestResult.Result));
        TestObject->SetNumberField(TEXT("executionTime"), TestResult.ExecutionTime);
        TestObject->SetStringField(TEXT("errorMessage"), TestResult.ErrorMessage);
        TestObject->SetStringField(TEXT("timestamp"), TestResult.Timestamp.ToString());
        
        TestResultsArray.Add(MakeShareable(new FJsonValueObject(TestObject)));
    }
    JsonObject->SetArrayField(TEXT("testResults"), TestResultsArray);
    
    // Performance metrics
    TArray<TSharedPtr<FJsonValue>> MetricsArray;
    for (const FQAPerformanceMetric& Metric : CurrentReportSession->PerformanceMetrics)
    {
        TSharedPtr<FJsonObject> MetricObject = MakeShareable(new FJsonObject);
        MetricObject->SetStringField(TEXT("metricName"), Metric.MetricName);
        MetricObject->SetStringField(TEXT("category"), Metric.Category);
        MetricObject->SetNumberField(TEXT("value"), Metric.Value);
        MetricObject->SetStringField(TEXT("unit"), Metric.Unit);
        MetricObject->SetNumberField(TEXT("threshold"), Metric.Threshold);
        MetricObject->SetBoolField(TEXT("withinThreshold"), Metric.bWithinThreshold);
        MetricObject->SetStringField(TEXT("timestamp"), Metric.Timestamp.ToString());
        
        MetricsArray.Add(MakeShareable(new FJsonValueObject(MetricObject)));
    }
    JsonObject->SetArrayField(TEXT("performanceMetrics"), MetricsArray);
    
    // Bug reports
    TArray<TSharedPtr<FJsonValue>> BugReportsArray;
    for (const FQABugReport& BugReport : CurrentReportSession->BugReports)
    {
        TSharedPtr<FJsonObject> BugObject = MakeShareable(new FJsonObject);
        BugObject->SetStringField(TEXT("title"), BugReport.Title);
        BugObject->SetStringField(TEXT("description"), BugReport.Description);
        BugObject->SetStringField(TEXT("severity"), UEnum::GetValueAsString(BugReport.Severity));
        BugObject->SetStringField(TEXT("category"), BugReport.Category);
        BugObject->SetStringField(TEXT("reproductionSteps"), BugReport.ReproductionSteps);
        BugObject->SetStringField(TEXT("expectedBehavior"), BugReport.ExpectedBehavior);
        BugObject->SetStringField(TEXT("actualBehavior"), BugReport.ActualBehavior);
        BugObject->SetStringField(TEXT("timestamp"), BugReport.Timestamp.ToString());
        
        BugReportsArray.Add(MakeShareable(new FJsonValueObject(BugObject)));
    }
    JsonObject->SetArrayField(TEXT("bugReports"), BugReportsArray);
    
    // Serialize to string
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    FString ReportPath = GetReportPath(TEXT("json"));
    if (FFileHelper::SaveStringToFile(JsonString, *ReportPath))
    {
        UE_LOG(LogQAReportGenerator, Log, TEXT("JSON report generated: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogQAReportGenerator, Error, TEXT("Failed to save JSON report: %s"), *ReportPath);
    }
}

void FQAReportGenerator::GenerateCSVReport()
{
    if (!CurrentReportSession.IsValid())
    {
        return;
    }

    FString CSVContent;
    
    // Header
    CSVContent += TEXT("Test Name,Category,Result,Execution Time (s),Error Message,Timestamp\n");
    
    // Test results
    for (const FQATestResult& TestResult : CurrentReportSession->TestResults)
    {
        CSVContent += FString::Printf(TEXT("\"%s\",\"%s\",\"%s\",%.3f,\"%s\",\"%s\"\n"),
            *TestResult.TestName,
            *TestResult.Category,
            *UEnum::GetValueAsString(TestResult.Result),
            TestResult.ExecutionTime,
            *TestResult.ErrorMessage.Replace(TEXT("\""), TEXT("\"\"\")), // Escape quotes
            *TestResult.Timestamp.ToString()
        );
    }
    
    FString ReportPath = GetReportPath(TEXT("csv"));
    if (FFileHelper::SaveStringToFile(CSVContent, *ReportPath))
    {
        UE_LOG(LogQAReportGenerator, Log, TEXT("CSV report generated: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogQAReportGenerator, Error, TEXT("Failed to save CSV report: %s"), *ReportPath);
    }
}

FString FQAReportGenerator::GenerateHTMLContent()
{
    if (!CurrentReportSession.IsValid())
    {
        return FString();
    }

    FString HTMLContent = TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
    HTMLContent += TEXT("<title>Transpersonal Game QA Report</title>\n");
    HTMLContent += TEXT("<style>\n");
    HTMLContent += TEXT("body { font-family: Arial, sans-serif; margin: 20px; }\n");
    HTMLContent += TEXT("h1 { color: #2c3e50; }\n");
    HTMLContent += TEXT("h2 { color: #34495e; border-bottom: 2px solid #ecf0f1; }\n");
    HTMLContent += TEXT("table { border-collapse: collapse; width: 100%; margin: 20px 0; }\n");
    HTMLContent += TEXT("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    HTMLContent += TEXT("th { background-color: #f2f2f2; }\n");
    HTMLContent += TEXT(".passed { color: #27ae60; }\n");
    HTMLContent += TEXT(".failed { color: #e74c3c; }\n");
    HTMLContent += TEXT(".warning { color: #f39c12; }\n");
    HTMLContent += TEXT(".skipped { color: #95a5a6; }\n");
    HTMLContent += TEXT(".summary { background-color: #ecf0f1; padding: 15px; border-radius: 5px; }\n");
    HTMLContent += TEXT("</style>\n");
    HTMLContent += TEXT("</head>\n<body>\n");
    
    // Header
    HTMLContent += FString::Printf(TEXT("<h1>Transpersonal Game QA Report</h1>\n"));
    HTMLContent += FString::Printf(TEXT("<div class=\"summary\">\n"));
    HTMLContent += FString::Printf(TEXT("<p><strong>Session:</strong> %s</p>\n"), *CurrentReportSession->SessionName);
    HTMLContent += FString::Printf(TEXT("<p><strong>Build Version:</strong> %s</p>\n"), *CurrentReportSession->BuildVersion);
    HTMLContent += FString::Printf(TEXT("<p><strong>Platform:</strong> %s</p>\n"), *CurrentReportSession->Platform);
    HTMLContent += FString::Printf(TEXT("<p><strong>Engine Version:</strong> %s</p>\n"), *CurrentReportSession->EngineVersion);
    HTMLContent += FString::Printf(TEXT("<p><strong>Start Time:</strong> %s</p>\n"), *CurrentReportSession->StartTime.ToString());
    HTMLContent += FString::Printf(TEXT("<p><strong>End Time:</strong> %s</p>\n"), *CurrentReportSession->EndTime.ToString());
    HTMLContent += FString::Printf(TEXT("<p><strong>Duration:</strong> %.2f seconds</p>\n"), CurrentReportSession->Duration.GetTotalSeconds());
    HTMLContent += TEXT("</div>\n");
    
    // Test Statistics
    HTMLContent += TEXT("<h2>Test Statistics</h2>\n");
    HTMLContent += TEXT("<table>\n");
    HTMLContent += TEXT("<tr><th>Metric</th><th>Count</th></tr>\n");
    HTMLContent += FString::Printf(TEXT("<tr><td>Total Tests</td><td>%d</td></tr>\n"), CurrentReportSession->TotalTests);
    HTMLContent += FString::Printf(TEXT("<tr><td class=\"passed\">Passed</td><td>%d</td></tr>\n"), CurrentReportSession->PassedTests);
    HTMLContent += FString::Printf(TEXT("<tr><td class=\"failed\">Failed</td><td>%d</td></tr>\n"), CurrentReportSession->FailedTests);
    HTMLContent += FString::Printf(TEXT("<tr><td class=\"warning\">Warnings</td><td>%d</td></tr>\n"), CurrentReportSession->WarningTests);
    HTMLContent += FString::Printf(TEXT("<tr><td class=\"skipped\">Skipped</td><td>%d</td></tr>\n"), CurrentReportSession->SkippedTests);
    HTMLContent += TEXT("</table>\n");
    
    // Test Results
    HTMLContent += TEXT("<h2>Test Results</h2>\n");
    HTMLContent += TEXT("<table>\n");
    HTMLContent += TEXT("<tr><th>Test Name</th><th>Category</th><th>Result</th><th>Execution Time</th><th>Error Message</th></tr>\n");
    
    for (const FQATestResult& TestResult : CurrentReportSession->TestResults)
    {
        FString ResultClass;
        switch (TestResult.Result)
        {
            case EQATestResult::Passed: ResultClass = TEXT("passed"); break;
            case EQATestResult::Failed: ResultClass = TEXT("failed"); break;
            case EQATestResult::Warning: ResultClass = TEXT("warning"); break;
            case EQATestResult::Skipped: ResultClass = TEXT("skipped"); break;
        }
        
        HTMLContent += FString::Printf(TEXT("<tr><td>%s</td><td>%s</td><td class=\"%s\">%s</td><td>%.3fs</td><td>%s</td></tr>\n"),
            *TestResult.TestName,
            *TestResult.Category,
            *ResultClass,
            *UEnum::GetValueAsString(TestResult.Result),
            TestResult.ExecutionTime,
            *TestResult.ErrorMessage
        );
    }
    HTMLContent += TEXT("</table>\n");
    
    // Performance Metrics
    if (CurrentReportSession->PerformanceMetrics.Num() > 0)
    {
        HTMLContent += TEXT("<h2>Performance Metrics</h2>\n");
        HTMLContent += TEXT("<table>\n");
        HTMLContent += TEXT("<tr><th>Metric</th><th>Category</th><th>Value</th><th>Unit</th><th>Threshold</th><th>Status</th></tr>\n");
        
        for (const FQAPerformanceMetric& Metric : CurrentReportSession->PerformanceMetrics)
        {
            FString StatusClass = Metric.bWithinThreshold ? TEXT("passed") : TEXT("failed");
            FString StatusText = Metric.bWithinThreshold ? TEXT("PASS") : TEXT("FAIL");
            
            HTMLContent += FString::Printf(TEXT("<tr><td>%s</td><td>%s</td><td>%.2f</td><td>%s</td><td>%.2f</td><td class=\"%s\">%s</td></tr>\n"),
                *Metric.MetricName,
                *Metric.Category,
                Metric.Value,
                *Metric.Unit,
                Metric.Threshold,
                *StatusClass,
                *StatusText
            );
        }
        HTMLContent += TEXT("</table>\n");
    }
    
    // Bug Reports
    if (CurrentReportSession->BugReports.Num() > 0)
    {
        HTMLContent += TEXT("<h2>Bug Reports</h2>\n");
        HTMLContent += TEXT("<table>\n");
        HTMLContent += TEXT("<tr><th>Title</th><th>Severity</th><th>Category</th><th>Description</th></tr>\n");
        
        for (const FQABugReport& BugReport : CurrentReportSession->BugReports)
        {
            HTMLContent += FString::Printf(TEXT("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n"),
                *BugReport.Title,
                *UEnum::GetValueAsString(BugReport.Severity),
                *BugReport.Category,
                *BugReport.Description
            );
        }
        HTMLContent += TEXT("</table>\n");
    }
    
    HTMLContent += TEXT("</body>\n</html>");
    
    return HTMLContent;
}

FString FQAReportGenerator::GetReportPath(const FString& Extension)
{
    if (!CurrentReportSession.IsValid())
    {
        return FString();
    }

    FString ReportDirectory = FPaths::ProjectSavedDir() / TEXT("QAReports");
    FString Timestamp = CurrentReportSession->StartTime.ToString(TEXT("%Y%m%d_%H%M%S"));
    FString FileName = FString::Printf(TEXT("%s_%s_%s.%s"), 
        *CurrentReportSession->SessionName, 
        *CurrentReportSession->BuildVersion,
        *Timestamp,
        *Extension
    );
    
    return ReportDirectory / FileName;
}