#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::Pass;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 CriticalCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float TotalExecutionTime;

    FQA_ValidationReport()
    {
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        CriticalCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFrameworkComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFrameworkComponent();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunStressTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport GetLastReport() const { return LastReport; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearTestResults();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    FQA_ValidationReport LastReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float TestTimeout;

    virtual void BeginPlay() override;

private:
    void AddTestCase(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void ValidateActorCount();
    void ValidateClassLoading();
    void ValidateSystemIntegration();
    void ValidatePerformanceMetrics();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void ExecuteFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void SchedulePeriodicTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UQA_TestFrameworkComponent* TestFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bRunTestsOnLevelStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float PeriodicTestInterval;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    FTimerHandle PeriodicTestTimer;
    void RunPeriodicValidation();
};