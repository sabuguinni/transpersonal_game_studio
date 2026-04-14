#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "QATestFramework.generated.h"

/**
 * QA Test Result Structure
 * Stores the result of a single test case
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test")
    bool bPassed;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test")
    FDateTime Timestamp;

    FQA_TestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

/**
 * QA Test Suite Structure
 * Groups related test cases together
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestSuite
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA Test Suite")
    FString SuiteName;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test Suite")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test Suite")
    int32 PassedTests;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test Suite")
    int32 FailedTests;

    UPROPERTY(BlueprintReadWrite, Category = "QA Test Suite")
    float TotalExecutionTime;

    FQA_TestSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Test Priority Levels
 */
UENUM(BlueprintType)
enum class EQA_TestPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

/**
 * QA Test Categories
 */
UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Physics         UMETA(DisplayName = "Physics"),
    Consciousness   UMETA(DisplayName = "Consciousness"),
    WorldGen        UMETA(DisplayName = "World Generation"),
    Environment     UMETA(DisplayName = "Environment"),
    Characters      UMETA(DisplayName = "Characters"),
    Animation       UMETA(DisplayName = "Animation"),
    AI              UMETA(DisplayName = "AI"),
    Combat          UMETA(DisplayName = "Combat"),
    Crowd           UMETA(DisplayName = "Crowd Simulation"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX"),
    Lighting        UMETA(DisplayName = "Lighting"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration")
};

/**
 * QA Test Framework Component
 * Main component for running automated tests
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_TestFrameworkComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFrameworkComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestSuite(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    // Test registration functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RegisterTest(const FString& TestName, EQA_TestCategory Category, EQA_TestPriority Priority);

    // Result functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    TArray<FQA_TestResult> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_TestSuite GetTestSuite(EQA_TestCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateMemoryUsage();

protected:
    // Test storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    TArray<FQA_TestSuite> TestSuites;

    UPROPERTY(BlueprintReadOnly, Category = "QA Framework")
    TArray<FQA_TestResult> AllTestResults;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bGenerateReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    FString ReportOutputPath;

private:
    // Internal test functions
    void RunCoreSystemTests();
    void RunPhysicsTests();
    void RunConsciousnessTests();
    void RunWorldGenerationTests();
    void RunEnvironmentTests();
    void RunCharacterTests();
    void RunAnimationTests();
    void RunAITests();
    void RunCombatTests();
    void RunCrowdTests();
    void RunAudioTests();
    void RunVFXTests();
    void RunLightingTests();
    void RunPerformanceTests();
    void RunIntegrationTests();

    // Helper functions
    FQA_TestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""));
    void AddTestResult(const FQA_TestResult& Result, EQA_TestCategory Category);
    void LogTestResult(const FQA_TestResult& Result);

    // Timing
    float CurrentTestStartTime;
    FDateTime TestSessionStartTime;
};

/**
 * QA Test Manager Actor
 * Manages the overall QA testing process
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Main QA functions
    UFUNCTION(BlueprintCallable, Category = "QA Manager", CallInEditor = true)
    void StartQAValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Manager", CallInEditor = true)
    void StopQAValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Manager", CallInEditor = true)
    void GenerateFullReport();

    UFUNCTION(BlueprintCallable, Category = "QA Manager")
    bool IsValidationRunning() const;

    UFUNCTION(BlueprintCallable, Category = "QA Manager")
    float GetValidationProgress() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Manager")
    UQA_TestFrameworkComponent* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bContinuousValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float ValidationInterval;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    bool bValidationRunning;

    UPROPERTY(BlueprintReadOnly, Category = "QA Status")
    float ValidationProgress;

private:
    FTimerHandle ValidationTimerHandle;
    void RunPeriodicValidation();
};