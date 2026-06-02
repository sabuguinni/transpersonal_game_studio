#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunModuleTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunDinosaurTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateTestReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 WarningTests;

private:
    void AddTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
};