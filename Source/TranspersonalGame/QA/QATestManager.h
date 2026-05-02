#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "QATestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotTested   UMETA(DisplayName = "Not Tested"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
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
        Result = EQA_TestResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DrawCalls = 0;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestManager : public AActor
{
    GENERATED_BODY()

public:
    AQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunMovementTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCombatTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAudioTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAITests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateDinosaurActors();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystems();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopPerformanceMonitoring();

    // Test reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void ExportTestResults();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bPerformanceMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float PerformanceTargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MaxAcceptableFrameTime;

private:
    float LastTestTime;
    bool bTestsRunning;

    // Helper functions
    void AddTestCase(const FString& TestName, const FString& Description);
    void UpdateTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);
    void CleanupDuplicateActors();
};