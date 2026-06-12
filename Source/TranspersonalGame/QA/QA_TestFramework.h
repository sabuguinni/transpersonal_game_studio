#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "QA_TestFramework.generated.h"

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
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Tests")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bLogResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float TestTimeout;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void AddTestCase(const FString& Name, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ClearAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float GetTotalExecutionTime() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "QA Framework")
    void OnTestCompleted(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintImplementableEvent, Category = "QA Framework")
    void OnAllTestsCompleted();

private:
    void ValidateVFXSystems();
    void ValidateAudioSystems();
    void ValidateCharacterSystems();
    void ValidateWorldGeneration();
    void ValidatePerformance();
    void ValidateIntegration();

    void LogTestResult(const FQA_TestCase& TestCase);
    void SetTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
};