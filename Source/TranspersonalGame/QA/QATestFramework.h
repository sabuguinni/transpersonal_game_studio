#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQATestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

UENUM(BlueprintType)
enum class EQATestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    Physics     UMETA(DisplayName = "Physics"),
    AI          UMETA(DisplayName = "AI Systems"),
    Environment UMETA(DisplayName = "Environment"),
    Character   UMETA(DisplayName = "Character"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "VFX"),
    Performance UMETA(DisplayName = "Performance"),
    Integration UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQATestResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQATestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQATestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString Details;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FDateTime Timestamp;

    FQATestResult()
    {
        TestName = TEXT("");
        Category = EQATestCategory::Core;
        Result = EQATestResult::NotRun;
        Details = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQATestCompleted, const FQATestResult&, TestResult);

/**
 * QA Test Framework Component
 * Provides automated testing capabilities for all game systems
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQATestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestCategory(EQATestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // Core system tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestCoreSystemsIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestAISystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestEnvironmentSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult TestPerformanceMetrics();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateFrameRate(float MinFPS = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateMemoryUsage(float MaxMemoryMB = 4096.0f);

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult ValidateAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQATestResult ValidateBlueprintCompilation();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQATestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "QA Testing")
    FOnQATestCompleted OnTestCompleted;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQATestResult> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bContinuousMonitoring;

private:
    float LastTestTime;
    bool bTestsRunning;

    // Helper functions
    FQATestResult CreateTestResult(const FString& TestName, EQATestCategory Category, EQATestResult Result, const FString& Details);
    void LogTestResult(const FQATestResult& Result);
    void AddTestResult(const FQATestResult& Result);
};

/**
 * QA Test Manager Actor
 * Manages all QA testing operations in the level
 */
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

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void InitializeQASystem();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void StopAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Management")
    void GenerateComprehensiveReport();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class UQATestFramework* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bRunTestsOnLevelStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float AutoTestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bGenerateDetailedLogs;
};