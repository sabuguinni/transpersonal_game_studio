#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
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
class TRANSPERSONALGAME_API UQA_TestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_TestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestByName(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTest(const FString& TestName, const FString& Description);

    // Results access
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "QA Testing")
    float GetOverallSuccessRate() const;

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestMemoryUsage();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCharacterSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestWorldGeneration();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bTestsRunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxDinosaurLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TargetFrameRate;

private:
    void ExecuteTest(const FString& TestName, TFunction<bool()> TestFunction);
    void LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message = TEXT(""));
    bool ValidateClassLoading();
    bool ValidateCDOConstruction();
    bool ValidateActorCounts();
    bool ValidatePerformanceMetrics();
};