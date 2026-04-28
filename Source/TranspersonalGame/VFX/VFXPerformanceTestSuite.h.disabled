// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "VFXPerformanceTestSuite.generated.h"

/**
 * Performance test suite for VFX systems
 * Validates frame rates, memory usage, and system stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFXPerformanceTestSuite : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXPerformanceTestSuite();

    // Performance test functions
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool RunFullPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool TestNiagaraSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool TestCreatureVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool TestEnvironmentalVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool TestVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool TestVFXLODSystem();

    // Stress testing
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool StressTestMultipleEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool StressTestParticleCount();

    // Performance metrics
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    float GetVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    int32 GetActiveParticleCount();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    float GetGPUUsagePercentage();

protected:
    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float MaxAcceptableMemoryMB = 512.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    int32 MaxParticleCount = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float MaxGPUUsagePercent = 80.0f;

    // Test results
    UPROPERTY(BlueprintReadOnly, Category = "Test Results")
    TArray<FString> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Test Results")
    bool bAllTestsPassed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Test Results")
    float LastTestDuration = 0.0f;

private:
    // Internal test functions
    bool ValidatePerformanceMetrics();
    void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = "");
    void ClearTestResults();
    
    // Performance monitoring
    float StartTime = 0.0f;
    float BaselineFPS = 0.0f;
    float BaselineMemory = 0.0f;
    
    // Test actors for spawning effects
    UPROPERTY()
    TArray<AActor*> TestActors;
    
    // Niagara systems for testing
    UPROPERTY()
    TArray<UNiagaraComponent*> TestNiagaraComponents;
};

/**
 * Automated VFX test runner
 * Runs comprehensive tests on VFX systems
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AVFXTestRunner : public AActor
{
    GENERATED_BODY()

public:
    AVFXTestRunner();

    // Automated testing
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunAutomatedVFXTests();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunPerformanceBenchmark();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunStabilityTest();

    // Test reporting
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    FString GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void SaveTestReport(const FString& Filename);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UVFXPerformanceTestSuite* PerformanceTestSuite;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    bool bRunTestsOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    float TestDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
    int32 TestIterations = 3;

    // Test results
    UPROPERTY(BlueprintReadOnly, Category = "Test Results")
    TArray<FString> ComprehensiveTestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Test Results")
    bool bTestsCompleted = false;

private:
    void RunSingleTestIteration();
    void CompileTestResults();
    
    FTimerHandle TestTimerHandle;
    int32 CurrentIteration = 0;
};

/**
 * VFX Quality Assurance utilities
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXQualityAssurance : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Quality validation functions
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static bool ValidateVFXSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static bool ValidateNiagaraAssets();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static bool ValidateVFXPerformanceSettings();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static TArray<FString> GetVFXSystemWarnings();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static TArray<FString> GetVFXSystemErrors();

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static float AnalyzeVFXPerformanceImpact();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static bool IsVFXSystemOptimized();

    // Automated fixes
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static bool AttemptAutomaticVFXOptimization();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    static void ApplyRecommendedVFXSettings();
};