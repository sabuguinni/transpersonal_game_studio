#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
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
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bIsCompiled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bIsFunctional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FQA_TestCase> TestCases;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsCompiled = false;
        bIsFunctional = false;
        ActorCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // System validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateDinosaurAI();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestActorCountLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void TestFrameRate();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestGameplayIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void LogTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void SaveTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void ClearTestResults();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogVerbose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Results")
    TArray<FQA_TestCase> AllTestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Results")
    TArray<FQA_SystemValidation> SystemValidations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Stats")
    int32 TotalTestsRun;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Stats")
    int32 TestsPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Stats")
    int32 TestsFailed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Stats")
    float LastTestRunTime;

private:
    float TestTimer;
    bool bTestsRunning;

    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void ExecuteTestCase(FQA_TestCase& TestCase, TFunction<bool()> TestFunction);
    void LogTestCase(const FQA_TestCase& TestCase);
    bool ValidateClassLoading(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
};