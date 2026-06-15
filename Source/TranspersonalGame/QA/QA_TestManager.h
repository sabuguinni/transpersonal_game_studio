#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Info
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Result = EQA_TestResult::Info;
        Description = TEXT("");
        ExecutionTime = 0.0f;
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

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunCharacterTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunWorldTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunDinosaurTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunIntegrationTests();

    // Test result management
    UFUNCTION(BlueprintCallable, Category = "QA")
    void AddTestResult(const FString& TestName, EQA_TestResult Result, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetPassCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetFailCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetOverallHealthScore() const;

    // Validation utilities
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateSystemIntegrity();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    bool bAutoRunTests;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float TestInterval;

private:
    float LastTestTime;
    bool bTestsRunning;

    // Internal test functions
    void TestCharacterSpawning();
    void TestDinosaurBehavior();
    void TestWorldGeneration();
    void TestLightingSetup();
    void TestAudioSystems();
    void TestUIElements();
    void TestSaveSystem();
    void TestNetworking();
    void TestMemoryUsage();
    void TestFrameRate();
};