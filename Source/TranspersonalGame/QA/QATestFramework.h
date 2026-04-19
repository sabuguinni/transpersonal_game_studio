#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Error       UMETA(DisplayName = "Error"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Functional      UMETA(DisplayName = "Functional"),
    Integration     UMETA(DisplayName = "Integration"),
    Performance     UMETA(DisplayName = "Performance"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Audio           UMETA(DisplayName = "Audio"),
    Visual          UMETA(DisplayName = "Visual"),
    Network         UMETA(DisplayName = "Network")
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
    int32 Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    bool bEnabled;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Category = EQA_TestCategory::Functional;
        Priority = 1;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        bEnabled = true;
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

    UPROPERTY(BlueprintReadOnly, Category = "Test Suite")
    int32 PassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Test Suite")
    int32 FailCount;

    UPROPERTY(BlueprintReadOnly, Category = "Test Suite")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        ErrorCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FQA_TestCase& TestCase, const FString& SuiteName = TEXT("Default"));

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RemoveTestCase(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearAllTests();

    // Results and reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetFailedTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallPassRate();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePropertyAccess(UObject* Object, const FString& PropertyName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TMap<FString, FQA_TestSuite> TestSuites;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bIsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString CurrentTestName;

private:
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateSuiteStatistics(FQA_TestSuite& Suite);
};