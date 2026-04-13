#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning"),
    Blocked     UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    Physics     UMETA(DisplayName = "Physics"),
    Performance UMETA(DisplayName = "Performance"),
    Assets      UMETA(DisplayName = "Assets"),
    Blueprints  UMETA(DisplayName = "Blueprints"),
    Gameplay    UMETA(DisplayName = "Gameplay"),
    Audio       UMETA(DisplayName = "Audio"),
    VFX         UMETA(DisplayName = "VFX"),
    AI          UMETA(DisplayName = "AI"),
    Integration UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FDateTime Timestamp;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Category = EQA_TestCategory::Core;
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FString ReportName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FDateTime GeneratedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    bool bBuildApproved;

    FQA_ValidationReport()
    {
        ReportName = TEXT("QA Validation Report");
        GeneratedAt = FDateTime::Now();
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        bBuildApproved = false;
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
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Core system tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestBlueprintCompilation();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveReportToFile(const FQA_ValidationReport& Report, const FString& FilePath);

    // Build approval
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ApproveBuild();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void BlockBuild(const FString& Reason);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bBuildBlocked;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString BlockReason;

private:
    void InitializeTestCases();
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description, EQA_TestCategory Category);
    void LogTestResult(const FQA_TestCase& TestCase);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    class UQA_TestFramework* TestFramework;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void DisplayTestResults();

public:
    virtual void Tick(float DeltaTime) override;
};