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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
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

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunModuleTests(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateActorSpawn(UClass* ActorClass, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateLevelAssets();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float MeasureFrameRate(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float MeasureMemoryUsage();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ExportTestResults(const FString& FilePath);

    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    int32 WarningTests;

private:
    // Internal test functions
    FQA_TestCase TestCoreModules();
    FQA_TestCase TestVFXSystems();
    FQA_TestCase TestCharacterSystems();
    FQA_TestCase TestWorldGeneration();
    FQA_TestCase TestAISystems();
    FQA_TestCase TestPerformance();

    void LogTestResult(const FQA_TestCase& TestCase);
    void ResetTestResults();
};