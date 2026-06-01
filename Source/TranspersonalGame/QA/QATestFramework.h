#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Skip
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::Skip;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
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
class TRANSPERSONALGAME_API UQATestFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQATestFramework();

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
    FQA_TestCase ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidatePerformanceMetrics();

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
    void UpdateTestSuiteStats(FQA_TestSuite& Suite);
};