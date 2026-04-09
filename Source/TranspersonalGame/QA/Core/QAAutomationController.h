#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "HAL/PlatformFilemanager.h"
#include "QAAutomationController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogQAAutomation, Log, All);

/**
 * Test execution status
 */
UENUM(BlueprintType)
enum class EQATestStatus : uint8
{
    NotStarted,
    Running,
    Passed,
    Failed,
    Skipped,
    Timeout
};

/**
 * Test result data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQATestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    EQATestStatus Status = EQATestStatus::NotStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    FString Category;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Test Result")
    TArray<FString> LogMessages;
};

/**
 * Test suite configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQATestSuiteConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> TestCategories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bRunPerformanceTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bRunFunctionalTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bRunStressTests = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TestTimeout = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bGenerateReport = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString ReportOutputPath;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestCompleted, const FQATestResult&, TestResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestSuiteCompleted, const TArray<FQATestResult>&, Results);

/**
 * QA Automation Controller
 * 
 * Central controller for managing and executing automated tests
 * in the Transpersonal Game project. Provides comprehensive test
 * orchestration, reporting, and validation capabilities.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQAAutomationController : public UObject
{
    GENERATED_BODY()

public:
    UQAAutomationController();

    /**
     * Initialize the automation controller
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void Initialize();

    /**
     * Run a specific test by name
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunTest(const FString& TestName);

    /**
     * Run all tests in a category
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunTestCategory(const FString& Category);

    /**
     * Run a complete test suite
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunTestSuite(const FQATestSuiteConfig& Config);

    /**
     * Stop all running tests
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void StopAllTests();

    /**
     * Get available test categories
     */
    UFUNCTION(BlueprintPure, Category = "QA Automation")
    TArray<FString> GetAvailableTestCategories() const;

    /**
     * Get available tests in a category
     */
    UFUNCTION(BlueprintPure, Category = "QA Automation")
    TArray<FString> GetTestsInCategory(const FString& Category) const;

    /**
     * Get test results
     */
    UFUNCTION(BlueprintPure, Category = "QA Automation")
    TArray<FQATestResult> GetTestResults() const { return TestResults; }

    /**
     * Get current test execution status
     */
    UFUNCTION(BlueprintPure, Category = "QA Automation")
    bool IsRunningTests() const { return bIsRunningTests; }

    /**
     * Generate test report
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    FString GenerateTestReport(bool bIncludeDetails = true);

    /**
     * Export test results to file
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool ExportTestResults(const FString& FilePath, const FString& Format = TEXT("JSON"));

    /**
     * Clear test results
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void ClearTestResults();

    /**
     * Run smoke tests (quick validation)
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunSmokeTests();

    /**
     * Run performance validation tests
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunPerformanceTests();

    /**
     * Run functional tests
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunFunctionalTests();

    /**
     * Run stress tests
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool RunStressTests();

    /**
     * Validate game build integrity
     */
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool ValidateBuildIntegrity();

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTestCompleted OnTestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTestSuiteCompleted OnTestSuiteCompleted;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRunningTests = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FQATestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FQATestSuiteConfig CurrentSuiteConfig;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentTestIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> PendingTests;

private:
    // Test execution management
    FTimerHandle TestExecutionTimer;
    FDateTime SuiteStartTime;
    
    /**
     * Execute next test in queue
     */
    void ExecuteNextTest();

    /**
     * Handle test completion
     */
    void OnTestExecutionCompleted(const FString& TestName, bool bSuccess, const FString& ErrorMessage);

    /**
     * Get all available automation tests
     */
    TArray<FString> GetAllAvailableTests() const;

    /**
     * Check if test exists
     */
    bool DoesTestExist(const FString& TestName) const;

    /**
     * Get test category from test name
     */
    FString GetTestCategory(const FString& TestName) const;

    /**
     * Create test result entry
     */
    FQATestResult CreateTestResult(const FString& TestName, EQATestStatus Status, 
                                  const FString& ErrorMessage = TEXT(""));

    /**
     * Generate HTML report
     */
    FString GenerateHTMLReport() const;

    /**
     * Generate JSON report
     */
    FString GenerateJSONReport() const;

    /**
     * Generate CSV report
     */
    FString GenerateCSVReport() const;

    /**
     * Calculate test statistics
     */
    void CalculateTestStatistics(int32& OutPassed, int32& OutFailed, int32& OutSkipped, 
                               float& OutTotalTime) const;
};