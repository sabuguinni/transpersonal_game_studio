#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"

DECLARE_LOG_CATEGORY_EXTERN(LogQAReportGenerator, Log, All);

/**
 * Enumeration for QA test results
 */
UENUM(BlueprintType)
enum class EQATestResult : uint8
{
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

/**
 * Enumeration for bug severity levels
 */
UENUM(BlueprintType)
enum class EQABugSeverity : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Cosmetic    UMETA(DisplayName = "Cosmetic")
};

/**
 * Structure representing a single test result
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQATestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Category;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQATestResult Result = EQATestResult::Skipped;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQATestResult()
    {
        Timestamp = FDateTime::Now();
    }
};

/**
 * Structure representing a performance metric
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQAPerformanceMetric
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString MetricName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Category;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float Value = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Unit;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float Threshold = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bWithinThreshold = true;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQAPerformanceMetric()
    {
        Timestamp = FDateTime::Now();
    }
};

/**
 * Structure representing a bug report
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQABugReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Title;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQABugSeverity Severity = EQABugSeverity::Medium;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Category;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ReproductionSteps;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ExpectedBehavior;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ActualBehavior;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQABugReport()
    {
        Timestamp = FDateTime::Now();
    }
};

/**
 * Structure representing a complete QA report session
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQAReportSession
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString SessionName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString BuildVersion;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Platform;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString EngineVersion;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FTimespan Duration;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FQATestResult> TestResults;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FQAPerformanceMetric> PerformanceMetrics;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FQABugReport> BugReports;

    // Statistics
    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 TotalTests = 0;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 PassedTests = 0;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 FailedTests = 0;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 WarningTests = 0;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 SkippedTests = 0;
};

/**
 * QA Report Generator
 * 
 * Generates comprehensive QA reports in multiple formats (HTML, JSON, CSV)
 * Tracks test results, performance metrics, and bug reports
 * Provides session-based reporting for organized test runs
 */
class TRANSPERSONALGAME_API FQAReportGenerator
{
public:
    FQAReportGenerator();
    ~FQAReportGenerator();

    // Session management
    void StartReportSession(const FString& SessionName, const FString& BuildVersion);
    void EndReportSession();
    bool IsGeneratingReport() const { return bIsGeneratingReport; }

    // Data collection
    void AddTestResult(const FQATestResult& TestResult);
    void AddPerformanceMetric(const FQAPerformanceMetric& Metric);
    void AddBugReport(const FQABugReport& BugReport);

    // Report generation
    void GenerateHTMLReport();
    void GenerateJSONReport();
    void GenerateCSVReport();

    // Getters
    TSharedPtr<FQAReportSession> GetCurrentSession() const { return CurrentReportSession; }

private:
    // Internal state
    bool bIsGeneratingReport;
    TSharedPtr<FQAReportSession> CurrentReportSession;

    // Report generation helpers
    FString GenerateHTMLContent();
    FString GetReportPath(const FString& Extension);
};

/**
 * Global QA Report Generator instance
 */
class TRANSPERSONALGAME_API FQAReportManager
{
public:
    static FQAReportGenerator& Get()
    {
        static FQAReportGenerator Instance;
        return Instance;
    }

    // Convenience methods for global access
    static void StartSession(const FString& SessionName, const FString& BuildVersion)
    {
        Get().StartReportSession(SessionName, BuildVersion);
    }

    static void EndSession()
    {
        Get().EndReportSession();
    }

    static void AddTest(const FQATestResult& TestResult)
    {
        Get().AddTestResult(TestResult);
    }

    static void AddMetric(const FQAPerformanceMetric& Metric)
    {
        Get().AddPerformanceMetric(Metric);
    }

    static void AddBug(const FQABugReport& BugReport)
    {
        Get().AddBugReport(BugReport);
    }
};

/**
 * Macros for easy QA reporting
 */
#define QA_START_SESSION(SessionName, BuildVersion) \
    FQAReportManager::StartSession(SessionName, BuildVersion)

#define QA_END_SESSION() \
    FQAReportManager::EndSession()

#define QA_REPORT_TEST(TestName, Category, Result, ExecutionTime, ErrorMessage) \
    do { \
        FQATestResult TestResult; \
        TestResult.TestName = TestName; \
        TestResult.Category = Category; \
        TestResult.Result = Result; \
        TestResult.ExecutionTime = ExecutionTime; \
        TestResult.ErrorMessage = ErrorMessage; \
        FQAReportManager::AddTest(TestResult); \
    } while(0)

#define QA_REPORT_METRIC(MetricName, Category, Value, Unit, Threshold) \
    do { \
        FQAPerformanceMetric Metric; \
        Metric.MetricName = MetricName; \
        Metric.Category = Category; \
        Metric.Value = Value; \
        Metric.Unit = Unit; \
        Metric.Threshold = Threshold; \
        Metric.bWithinThreshold = (Value <= Threshold); \
        FQAReportManager::AddMetric(Metric); \
    } while(0)

#define QA_REPORT_BUG(Title, Description, Severity, Category) \
    do { \
        FQABugReport BugReport; \
        BugReport.Title = Title; \
        BugReport.Description = Description; \
        BugReport.Severity = Severity; \
        BugReport.Category = Category; \
        FQAReportManager::AddBug(BugReport); \
    } while(0)