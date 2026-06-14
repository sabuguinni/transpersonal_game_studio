#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

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
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    float TestTimeout;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateCoreSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsSystemHealthy();

protected:
    virtual void BeginPlay() override;

private:
    void InitializeTestSuites();
    void LogTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
};