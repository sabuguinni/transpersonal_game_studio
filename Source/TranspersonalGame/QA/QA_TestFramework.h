#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped"),
    InProgress  UMETA(DisplayName = "In Progress")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    VFX         UMETA(DisplayName = "Visual Effects"),
    Audio       UMETA(DisplayName = "Audio Systems"),
    Performance UMETA(DisplayName = "Performance"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    Integration UMETA(DisplayName = "Integration"),
    Regression  UMETA(DisplayName = "Regression")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    bool bIsAutomated;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::InProgress;
        ErrorMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
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
    float TotalExecutionTimeMs;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTimeMs = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void InitializeTestFramework();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawning(UClass* ActorClass, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAssetLoading(const FString& AssetPath);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float MeasureFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float MeasureMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformanceThresholds();

    // VFX specific testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateNiagaraEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateParticlePerformance();

    // Audio testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateSoundEffects();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestSuite GetTestSuiteResults(const FString& SuiteName);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TMap<FString, FQA_TestCase> RegisteredTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float PerformanceThresholdFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float PerformanceThresholdMemoryMB;

private:
    void InitializeDefaultTests();
    void RunCoreSystemTests();
    void RunVFXTests();
    void RunAudioTests();
    void RunPerformanceTests();
    void RunIntegrationTests();
    
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category);
    void UpdateTestResult(FQA_TestCase& TestCase, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);
};