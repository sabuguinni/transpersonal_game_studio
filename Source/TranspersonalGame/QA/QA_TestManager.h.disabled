#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Warning     UMETA(DisplayName = "Warning")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestDescription;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float TimeoutSeconds;

    FQA_TestCase()
    {
        TestName = TEXT("Unnamed Test");
        TestDescription = TEXT("No description");
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        TimeoutSeconds = 30.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test Management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    // Core System Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestActorMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestCharacterController();

    // Performance Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestMemoryUsage();

    // Integration Tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestCrossSystemIntegration();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    int32 PassedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    int32 FailedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    int32 WarningTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    float LastTestRunTime;

private:
    void InitializeTestCases();
    void LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message = TEXT(""));
    bool IsValidBiomeLocation(const FVector& Location, EBiomeType BiomeType);
    
    FTimerHandle TestTimerHandle;
    bool bTestsRunning;
};