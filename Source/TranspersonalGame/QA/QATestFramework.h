#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

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

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    bool ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void GenerateTestReport();

    // Test result accessors
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    float GetOverallSuccessRate() const;

protected:
    virtual void BeginPlay() override;

private:
    void AddTestCase(const FString& Name, const FString& Description);
    void UpdateTestResult(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
    bool TestActorSpawning();
    bool TestModuleLoading();
    bool TestVFXIntegration();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

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