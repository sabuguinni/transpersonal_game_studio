#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "QATestManager.generated.h"

/**
 * QA Test Manager - Automated testing and validation system
 * Runs comprehensive tests on all game systems and reports results
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunSystemTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool RunGameplayTests();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateMinPlayableMap();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystems();

    // Test result properties
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bAllTestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 TestsRun;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 TestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 TestsFailed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    float LastTestDuration;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FString> FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FString> TestLog;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ComponentCount;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogVerbose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bStopOnFirstFailure;

private:
    // Internal test functions
    bool TestActorSpawning();
    bool TestComponentSystems();
    bool TestGameModeIntegration();
    bool TestPlayerCharacter();
    bool TestDinosaurAI();
    bool TestAudioPlayback();
    bool TestVFXSpawning();
    bool TestPerformanceMetrics();
    bool TestMemoryUsage();
    bool TestFrameRate();

    // Helper functions
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    void ResetTestResults();
    void UpdatePerformanceMetrics();

    // Internal state
    float TimeSinceLastTest;
    bool bTestsRunning;
    int32 CurrentTestIndex;
    TArray<FString> TestQueue;
};