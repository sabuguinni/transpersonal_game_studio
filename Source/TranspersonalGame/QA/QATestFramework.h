#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQATestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Warning     UMETA(DisplayName = "Warning"),
    Failed      UMETA(DisplayName = "Failed"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQATestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQATestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQATestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQATestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQATestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    TArray<FQATestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Suite")
    int32 CriticalIssues;

    FQATestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        CriticalIssues = 0;
    }
};

/**
 * QA Test Framework Component
 * Provides automated testing capabilities for the Transpersonal Game
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQATestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test Suite Management
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void InitializeTestSuites();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestSuite GetTestSuiteResults(const FString& SuiteName);

    // Individual Test Functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestConsciousnessSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestBlueprintCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQATestCase TestNPCBehavior();

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidatePerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateAssetOptimization();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FString GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void SaveQAReport(const FString& ReportContent);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool IsQAGateApproved();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TMap<FString, FQATestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    int32 MaxCriticalIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float PerformanceTargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float MaxMemoryUsageMB;

private:
    float LastTestTime;
    bool bTestsInitialized;

    // Helper functions
    FQATestCase CreateTestCase(const FString& Name, const FString& Description);
    void LogTestResult(const FQATestCase& TestCase);
    bool CheckClassExists(const FString& ClassName);
    float MeasureExecutionTime(TFunction<void()> TestFunction);
};