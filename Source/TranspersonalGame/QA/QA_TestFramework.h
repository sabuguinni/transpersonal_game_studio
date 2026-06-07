#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
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
        Result = EQA_TestResult::NotRun;
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

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestByName(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateActorExists(const FString& ActorName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateComponentExists(AActor* Actor, const FString& ComponentName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateClassLoads(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateMapLoads(const FString& MapPath);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float MeasureFrameRate(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    float MeasureMemoryUsage();

    // System validation
    UFUNCTION(BlueprintCallable, Category = "QA System")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA System")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA System")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA System")
    bool ValidateWorldGeneration();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bIsRunningTests;

private:
    void LogTestResult(const FQA_TestCase& TestCase);
    float StartTime;
};