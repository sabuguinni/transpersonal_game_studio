#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    VFX         UMETA(DisplayName = "Visual Effects"),
    Audio       UMETA(DisplayName = "Audio Systems"),
    Performance UMETA(DisplayName = "Performance"),
    Integration UMETA(DisplayName = "Integration"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    UI          UMETA(DisplayName = "User Interface"),
    Network     UMETA(DisplayName = "Networking")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    bool bIsAutomated;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bIsAutomated = true;
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
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

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

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAssetExists(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorInLevel(const FString& ActorName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformanceMetrics(float MaxFrameTime, int32 MaxDrawCalls);

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

    // Getters
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestSuite> GetAllTestSuites() const { return TestSuites; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestSuite(const FString& SuiteName) const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetTotalTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallPassRate() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    bool bAutoRunOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    bool bGenerateDetailedLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Data")
    FString LogOutputPath;

private:
    // Internal test execution
    EQA_TestResult ExecuteTest(const FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& TestSuite);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FString> TestSuitesToRun;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartQATests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StopQATests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GetTestStatus() const;
};