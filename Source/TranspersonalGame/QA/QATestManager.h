#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "QATestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TestsFailed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        TestsPassed = 0;
        TestsFailed = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunSystemTests(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult RunSingleTest(const FString& TestName);

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateNarrativeSystem();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    float MeasureFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 CountActorsInLevel();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool CheckMemoryUsage();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool TestDinosaurAI();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FString GetLastTestReport() const { return LastTestReport; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString LastTestReport;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bTestsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float TestStartTime;

private:
    // Internal helper functions
    FQA_TestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_TestResult& Result);
    bool IsClassLoaded(const FString& ClassName);
    bool IsActorTypePresent(const FString& ActorType);
};