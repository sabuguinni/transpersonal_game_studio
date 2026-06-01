#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Skipped
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString SuiteName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 WarningCount;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RegisterTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void AddTestCase(const FString& SuiteName, const FString& TestName, const FString& Description);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateModuleClasses();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateGameMode();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateWorldGeneration();

    // Performance tests
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase TestFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase TestMemoryUsage();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FString GetTestSummary();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestSuite> GetAllTestSuites() const { return TestSuites; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bIsRunningTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float LastTestRunTime;

private:
    void InitializeDefaultTests();
    FQA_TestCase CreateTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);
};