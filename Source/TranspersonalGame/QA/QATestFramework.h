#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Misc/AutomationTest.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Info        UMETA(DisplayName = "Info")
};

UENUM(BlueprintType)
enum class EQA_QATestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    Physics     UMETA(DisplayName = "Physics"),
    AI          UMETA(DisplayName = "AI Systems"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "Visual Effects"),
    Performance UMETA(DisplayName = "Performance"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    UI          UMETA(DisplayName = "User Interface"),
    Network     UMETA(DisplayName = "Networking"),
    Memory      UMETA(DisplayName = "Memory")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_QATestResult_07D
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQA_QATestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString BuildVersion;

    FQA_QATestResult_07D()
    {
        TestName = TEXT("");
        Category = EQA_QATestCategory::Core;
        Severity = EQATestSeverity::Medium;
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeout;

    // Core test functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestsByCategory(EQA_QATestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D RunSingleTest(const FString& TestName);

    // Core system tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestCoreSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestAIBehaviorTrees();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestAudioSubsystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestVFXSystems();

    // Performance tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestPerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQAPerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateFrameRate(float TargetFPS = 60.0f);

    // Gameplay tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestDinosaurSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestPlayerSurvivalMechanics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestWorldGeneration();

    // Memory tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestMemoryLeaks();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D TestGarbageCollection();

    // Stress tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D StressTestActorSpawning(int32 ActorCount = 1000);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_QATestResult_07D StressTestAIBehavior(int32 AICount = 100);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    // Test result accessors
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_QATestResult_07D> GetTestHistory();

    // Bug tracking integration
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ReportBug(const FString& BugDescription, EQATestSeverity Severity, EQA_QATestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void MarkBugFixed(const FString& BugID);

protected:
    UPROPERTY()
    TArray<FQA_QATestResult_07D> TestResults;

    UPROPERTY()
    TArray<FQAPerformanceMetrics> PerformanceHistory;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float PerformanceTestDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxStressTestActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTestsOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedReports;

private:
    // Internal test helpers
    FQA_QATestResult_07D CreateTestResult(const FString& TestName, EQA_QATestCategory Category, 
                                  EQATestSeverity Severity, bool bPassed, 
                                  const FString& ErrorMessage = TEXT(""));

    void LogTestResult(const FQA_QATestResult_07D& Result);
    
    bool ValidateSystemRequirements();
    
    void InitializeTestEnvironment();
    
    void CleanupTestEnvironment();

    // Performance monitoring
    FQAPerformanceMetrics CapturePerformanceSnapshot();
    
    void StartPerformanceMonitoring();
    
    void StopPerformanceMonitoring();

    // Test data
    FDateTime TestSessionStartTime;
    
    bool bTestingInProgress;
    
    int32 TotalTestsRun;
    
    int32 TestsPassed;
    
    int32 TestsFailed;
};

/**
 * Automated Test Macros for Prehistoric Game Testing
 */
#define PREHISTORIC_TEST_INIT(TestName, Category) \
    FQA_QATestResult_07D TestResult; \
    TestResult.TestName = TestName; \
    TestResult.Category = Category; \
    TestResult.Timestamp = FDateTime::Now(); \
    float StartTime = FPlatformTime::Seconds();

public:
    AQA_TestActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Test Actor")
    class UQA_TestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test Actor")
    bool bRunTestsOnBeginPlay;

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void StartQATests();

protected:
    virtual void BeginPlay() override;
};