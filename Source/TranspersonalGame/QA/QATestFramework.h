// Copyright Transpersonal Game Studio. All Rights Reserved.
// QATestFramework.h - Comprehensive QA testing framework for prehistoric survival game

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Misc/AutomationTest.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQATestSeverity : uint8
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

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString TestName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQA_QATestCategory Category;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQATestSeverity Severity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bPassed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
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
        Timestamp = FDateTime::Now();
        BuildVersion = TEXT("Unknown");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQAPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float FrameTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float GameThreadTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float RenderThreadTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float GPUTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 DrawCalls;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Triangles;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float MemoryUsage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 ActiveActors;

    FQAPerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsage = 0.0f;
        ActiveActors = 0;
    }
};

/**
 * QA Test Framework Subsystem
 * Provides comprehensive testing capabilities for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQATestFramework();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
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

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestResults(const FString& FilePath);

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

#define PREHISTORIC_TEST_PASS(Message) \
    TestResult.bPassed = true; \
    TestResult.ErrorMessage = Message; \
    TestResult.ExecutionTime = FPlatformTime::Seconds() - StartTime; \
    return TestResult;

#define PREHISTORIC_TEST_FAIL(Message) \
    TestResult.bPassed = false; \
    TestResult.ErrorMessage = Message; \
    TestResult.ExecutionTime = FPlatformTime::Seconds() - StartTime; \
    UE_LOG(LogTemp, Error, TEXT("QA Test Failed: %s - %s"), *TestResult.TestName, *Message); \
    return TestResult;

#define PREHISTORIC_TEST_VALIDATE(Condition, FailMessage) \
    if (!(Condition)) { \
        PREHISTORIC_TEST_FAIL(FailMessage); \
    }

/**
 * Performance Validation Macros
 */
#define VALIDATE_FRAME_TIME(MaxFrameTime) \
    if (GetCurrentPerformanceMetrics().FrameTime > MaxFrameTime) { \
        PREHISTORIC_TEST_FAIL(FString::Printf(TEXT("Frame time %.2fms exceeds limit %.2fms"), \
                              GetCurrentPerformanceMetrics().FrameTime, MaxFrameTime)); \
    }

#define VALIDATE_MEMORY_USAGE(MaxMemoryMB) \
    if (GetCurrentPerformanceMetrics().MemoryUsage > MaxMemoryMB) { \
        PREHISTORIC_TEST_FAIL(FString::Printf(TEXT("Memory usage %.2fMB exceeds limit %.2fMB"), \
                              GetCurrentPerformanceMetrics().MemoryUsage, MaxMemoryMB)); \
    }