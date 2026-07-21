#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Skipped
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
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

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bIsSystemHealthy;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FString> HealthIssues;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        ActorCount = 0;
        ComponentCount = 0;
        bIsSystemHealthy = true;
    }
};

/**
 * QA Test Manager - Comprehensive testing and validation system
 * Validates all game systems, performance metrics, and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestManager : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestManager();

    // Core Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunRegressionTests();

    // Specific System Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckFrameRate();

    // Integration Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPlayerSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXTriggers();

    // Regression Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckDegenerateLabels();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CheckEssentialActors();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<AActor*> GetActorsByLabel(const FString& LabelFilter);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_SystemValidation AnalyzeSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResults();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA Testing")
    float TargetFrameRate;

private:
    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void AddTestResult(const FQA_TestCase& TestCase);
    float MeasureExecutionTime(TFunction<void()> TestFunction);
    bool IsActorHealthy(AActor* Actor);
    void ValidateActorTransform(AActor* Actor, FQA_TestCase& TestCase);
};