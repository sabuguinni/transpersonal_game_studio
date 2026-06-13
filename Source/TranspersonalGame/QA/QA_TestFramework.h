#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    VFX             UMETA(DisplayName = "VFX"),
    Audio           UMETA(DisplayName = "Audio"),
    AI              UMETA(DisplayName = "AI"),
    Physics         UMETA(DisplayName = "Physics")
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
        Category = EQA_TestCategory::Compilation;
        Result = EQA_TestResult::NotRun;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ComponentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderTime;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
        ActorCount = 0;
        ComponentCount = 0;
        RenderTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestsByCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunSingleTest(const FString& TestName);

    // Test registration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RegisterTest(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearAllTests();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformanceThresholds();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestVFXSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestAISystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestPhysicsSystemIntegration();

    // Compilation testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassCompilation(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FString> GetFailedCompilationClasses();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestCases; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FQA_PerformanceMetrics LastPerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float PerformanceFrameRateThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float PerformanceMemoryThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorCountThreshold;

private:
    // Internal test execution
    bool ExecuteCompilationTest(const FString& TestName);
    bool ExecutePerformanceTest(const FString& TestName);
    bool ExecuteIntegrationTest(const FString& TestName);
    bool ExecuteGameplayTest(const FString& TestName);

    // Utility functions
    void LogTestResult(const FQA_TestCase& TestCase);
    FString GetTestCategoryString(EQA_TestCategory Category);
    FString GetTestResultString(EQA_TestResult Result);
};