#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQATestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EQATestCategory : uint8
{
    Performance     UMETA(DisplayName = "Performance"),
    Functionality   UMETA(DisplayName = "Functionality"),
    Integration     UMETA(DisplayName = "Integration"),
    Memory          UMETA(DisplayName = "Memory"),
    Assets          UMETA(DisplayName = "Assets"),
    Blueprints      UMETA(DisplayName = "Blueprints"),
    Audio           UMETA(DisplayName = "Audio"),
    VFX             UMETA(DisplayName = "VFX")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQATestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQATestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQATestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FDateTime Timestamp;

    FQATestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Category = EQATestCategory::Functionality;
        Result = EQATestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQAValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FDateTime ReportTimestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    TArray<FQATestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 CriticalTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    bool bBuildBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString BlockReason;

    FQAValidationReport()
    {
        CycleID = TEXT("");
        ReportTimestamp = FDateTime::Now();
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        CriticalTests = 0;
        bBuildBlocked = false;
        BlockReason = TEXT("");
    }
};

/**
 * QA Test Framework Component
 * Provides comprehensive testing and validation for all game systems
 * Agent #18 - QA & Testing Agent
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQATestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core QA Functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunTestCategory(EQATestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQAValidationReport GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void BlockBuild(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void UnblockBuild();

    // Performance Testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestActorCount();

    // System Integration Testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestComponentSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestAssetLoading();

    // Blueprint Testing
    UFUNCTION(BlueprintCallable, Category = "QA Blueprints")
    void TestBlueprintCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Blueprints")
    void ValidateBlueprintReferences();

    // Audio/VFX Testing
    UFUNCTION(BlueprintCallable, Category = "QA Audio")
    void TestAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    void TestVFXSystems();

    // Getters
    UFUNCTION(BlueprintPure, Category = "QA Framework")
    const TArray<FQATestCase>& GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintPure, Category = "QA Framework")
    bool IsBuildBlocked() const { return bBuildBlocked; }

    UFUNCTION(BlueprintPure, Category = "QA Framework")
    FString GetBlockReason() const { return BlockReason; }

protected:
    // Test execution helpers
    void ExecuteTest(const FString& TestName, const FString& Description, EQATestCategory Category, TFunction<bool()> TestFunction);
    void LogTestResult(const FQATestCase& TestCase);
    void UpdateTestStatistics();

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxActorCount = 10000;

    // Test data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    TArray<FQATestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Status")
    bool bBuildBlocked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Status")
    FString BlockReason;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Statistics")
    int32 TotalTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Statistics")
    int32 PassedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Statistics")
    int32 FailedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Statistics")
    int32 WarningTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Statistics")
    int32 CriticalTests;

    // Test timing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Timing")
    float LastTestExecutionTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Timing")
    FDateTime LastTestTimestamp;
};