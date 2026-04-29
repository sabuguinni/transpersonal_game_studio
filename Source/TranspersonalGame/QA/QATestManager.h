#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../Core/SharedTypes.h"
#include "QATestManager.generated.h"

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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
    FDateTime ReportTimestamp;

    FQA_ValidationReport()
    {
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
        ReportTimestamp = FDateTime::Now();
    }
};

/**
 * QA Test Manager - Validates game systems and reports issues
 * Runs automated tests on all game modules and components
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQATestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunModuleTests(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport GetLastReport() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Specific test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestAISystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCombatSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSystems();

    // Validation helpers
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorInLevel(TSubclassOf<AActor> ActorClass, const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateComponentOnActor(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass, const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""));

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_ValidationReport> ReportHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnStartup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxReportHistory;

private:
    void AddTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void StartTest(const FString& TestName);
    void FinishCurrentTest(bool bPassed, const FString& ErrorMessage = TEXT(""));
    
    FString CurrentTestName;
    double TestStartTime;
};