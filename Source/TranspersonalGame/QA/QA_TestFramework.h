#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Functionality   UMETA(DisplayName = "Functionality"),
    VFX            UMETA(DisplayName = "VFX"),
    Audio          UMETA(DisplayName = "Audio"),
    AI             UMETA(DisplayName = "AI"),
    Physics        UMETA(DisplayName = "Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::Skipped;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
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
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        TotalExecutionTime = 0.0f;
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
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTestSuite(const FQA_TestSuite& TestSuite);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateClassCompilation(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateActorSpawning(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystem(const FString& SystemPath);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystem(const FString& AudioPath);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float MeasureFrameRate(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float MeasureMemoryUsage();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestAudioIntegration();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void SaveTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void LogTestResults();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    FQA_TestSuite CurrentTestSuite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeout;

private:
    // Internal helper functions
    void InitializeTestFramework();
    void SetupDefaultTestSuites();
    FQA_TestCase CreateTestCase(const FString& Name, EQA_TestCategory Category, const FString& Description);
    void ExecuteTestCase(FQA_TestCase& TestCase);
    void UpdateTestSuiteStats(FQA_TestSuite& Suite);
};