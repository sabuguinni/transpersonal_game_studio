#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Functionality   UMETA(DisplayName = "Functionality"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Assets          UMETA(DisplayName = "Assets"),
    Memory          UMETA(DisplayName = "Memory")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FDateTime Timestamp;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 ErrorCount;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        ErrorCount = 0;
    }
};

/**
 * QA Test Framework Component
 * Provides automated testing capabilities for the Transpersonal Game
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_TestFrameworkComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFrameworkComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test registration functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Result functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestResults(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestSuite> GetAllTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float MeasureFrameTime();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetMemoryUsage();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bLogResults;

private:
    float LastTestTime;
    
    // Core test implementations
    void RunCompilationTests();
    void RunFunctionalityTests();
    void RunPerformanceTests();
    void RunIntegrationTests();
    void RunGameplayTests();
    void RunAssetTests();
    void RunMemoryTests();

    // Helper functions
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category);
};

/**
 * QA Test Manager Actor
 * Manages and coordinates all QA testing activities
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Manager functions
    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void InitializeTestFramework();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void StartContinuoustesting();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void StopContinuousTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void ExportTestResults(const FString& FilePath);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UQA_TestFrameworkComponent* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bContinuousTesting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ContinuousTestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoGenerateReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    FString ReportOutputPath;

private:
    float LastReportTime;
    bool bTestingInitialized;
};