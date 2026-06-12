#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
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

    // Core testing methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCharacterTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunWorldTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    // Test result management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetTestSuccessRate() const;

    // Validation utilities
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorCount(int32 MaxActors = 8000);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassCompilation(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateNiagaraComponents();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    float TotalTestTime;

    // Internal test methods
    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
    void ClearTestResults();
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
};