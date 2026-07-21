#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "QA_VFXIntegrationTester.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotTested       UMETA(DisplayName = "Not Tested"),
    Pass           UMETA(DisplayName = "Pass"),
    Fail           UMETA(DisplayName = "Fail"),
    Warning        UMETA(DisplayName = "Warning"),
    Critical       UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    FQA_VFXTestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_VFXTestResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_VFXPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveNiagaraSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActiveParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceWithinLimits;

    FQA_VFXPerformanceMetrics()
    {
        ActiveNiagaraSystems = 0;
        ActiveParticles = 0;
        GPUMemoryUsage = 0.0f;
        FrameTime = 0.0f;
        bPerformanceWithinLimits = true;
    }
};

/**
 * QA VFX Integration Tester - Validates VFX systems integration with audio and gameplay
 * Tests Niagara effects, audio synchronization, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXIntegrationTester : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXIntegrationTester();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TestMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* FootstepVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* CampfireVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    int32 MaxTestIterations;

    // Test Results
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    TArray<FQA_VFXTestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    FQA_VFXPerformanceMetrics PerformanceMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    bool bAllTestsPassed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    int32 TotalTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    int32 PassedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Results")
    int32 FailedTests;

public:
    // Test Execution Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunFootstepVFXTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunCampfireVFXTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunAudioSyncTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void ClearTestResults();

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateNiagaraSystem(UNiagaraComponent* NiagaraComp, const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAudioComponent(UAudioComponent* AudioComp, const FString& AudioName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_VFXPerformanceMetrics GatherPerformanceMetrics();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "QA Utilities")
    void LogTestResult(const FQA_VFXTestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Utilities")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Utilities")
    FString GetTestSummary();

private:
    FTimerHandle TestTimerHandle;
    int32 CurrentTestIteration;

    void ExecuteTestIteration();
    void AddTestResult(const FString& TestName, const FString& Description, EQA_VFXTestResult Result, const FString& ErrorMsg = TEXT(""));
    void UpdateTestStatistics();
};