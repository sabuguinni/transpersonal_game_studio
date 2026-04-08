#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Core/VFX/VFXSystemManager.h"
#include "VFXTestSuite.generated.h"

/**
 * VFX Test Suite - Comprehensive testing for the VFX system
 * Tests Niagara system spawning, performance, memory usage, and visual quality
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AVFXTestActor : public AActor
{
    GENERATED_BODY()

public:
    AVFXTestActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    bool bRunPerformanceTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    bool bRunMemoryTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    bool bRunVisualTests = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    bool bRunStressTests = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    int32 StressTestVFXCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    float TestDuration = 30.0f;

    // Test Results
    UPROPERTY(BlueprintReadOnly, Category = "VFX Testing")
    TMap<FString, float> PerformanceResults;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Testing")
    TMap<FString, int32> MemoryResults;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Testing")
    TArray<FString> TestErrors;

    UPROPERTY(BlueprintReadOnly, Category = "VFX Testing")
    bool bTestsCompleted = false;

    // Test Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void StartVFXTests();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void StopVFXTests();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunMemoryTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunVisualQualityTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunStressTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunVFXSpawnTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunVFXPoolingTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void RunVFXLODTest();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    void GenerateTestReport();

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool ValidateNiagaraSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "VFX Testing")
    bool ValidateVFXMemoryUsage();

protected:
    UPROPERTY()
    UVFXSystemManager* VFXManager;

    UPROPERTY()
    TArray<UNiagaraComponent*> TestVFXComponents;

    UPROPERTY()
    float TestStartTime;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    int32 FrameCount;

    UPROPERTY()
    float TotalFrameTime;

    // Internal test methods
    void LogTestResult(const FString& TestName, bool bSuccess, const FString& Details = "");
    void MeasureFrameTime();
    void CheckMemoryUsage();
    void ValidateVFXComponent(UNiagaraComponent* Component, EVFXType ExpectedType);
    void CleanupTestVFX();
};

/**
 * VFX Automation Tests - Unit tests for the VFX system
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemBasicTest, "TranspersonalGame.VFX.BasicFunctionality", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemPerformanceTest, "TranspersonalGame.VFX.Performance", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemMemoryTest, "TranspersonalGame.VFX.Memory", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemStressTest, "TranspersonalGame.VFX.StressTest", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemPoolingTest, "TranspersonalGame.VFX.Pooling", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXSystemLODTest, "TranspersonalGame.VFX.LOD", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * VFX Quality Assurance Manager - Manages all VFX testing and validation
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UVFXQAManager : public UObject
{
    GENERATED_BODY()

public:
    UVFXQAManager();

    // QA Test Management
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void RunFullVFXTestSuite();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void RunQuickVFXValidation();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void RunPerformanceBenchmark();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void RunMemoryProfileTest();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void RunVisualRegressionTest();

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX QA Config")
    bool bEnableAutomaticTesting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX QA Config")
    float TestInterval = 60.0f; // Run tests every minute

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX QA Config")
    int32 MaxAllowedVFXCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX QA Config")
    float MaxAllowedFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX QA Config")
    int32 MaxAllowedMemoryMB = 512;

    // Test Results
    UPROPERTY(BlueprintReadOnly, Category = "VFX QA Results")
    TMap<FString, bool> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "VFX QA Results")
    TArray<FString> QAErrors;

    UPROPERTY(BlueprintReadOnly, Category = "VFX QA Results")
    TArray<FString> QAWarnings;

    UPROPERTY(BlueprintReadOnly, Category = "VFX QA Results")
    float LastTestDuration;

    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    bool ValidateAllVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    bool ValidateVFXPerformanceTargets();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    bool ValidateVFXMemoryTargets();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    bool ValidateVFXVisualQuality();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void ExportTestResults(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "VFX QA")
    void LogQAResults();

protected:
    UPROPERTY()
    TArray<AVFXTestActor*> TestActors;

    UPROPERTY()
    float LastTestTime;

    // Internal validation methods
    bool ValidateVFXSystemIntegrity();
    bool ValidateNiagaraSystemReferences();
    bool ValidateVFXPooling();
    bool ValidateVFXLODSystem();
    void CollectPerformanceMetrics();
    void CollectMemoryMetrics();
    void AnalyzeTestResults();
};

/**
 * VFX Debug Visualizer - Visual debugging tools for VFX
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API AVFXDebugVisualizer : public AActor
{
    GENERATED_BODY()

public:
    AVFXDebugVisualizer();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Debug Visualization
    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void EnableVFXDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void DisableVFXDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void ToggleVFXDebugVisualization();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void ShowVFXBounds();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void ShowVFXPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void ShowVFXMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "VFX Debug")
    void ShowVFXLODInfo();

    // Debug Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    bool bShowVFXBounds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    bool bShowVFXStats = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    bool bShowVFXMemory = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    bool bShowVFXLOD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    FColor DebugBoundsColor = FColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Debug")
    FColor DebugStatsColor = FColor::Green;

protected:
    UPROPERTY()
    UVFXSystemManager* VFXManager;

    void DrawVFXDebugInfo();
    void DrawVFXBounds(UNiagaraComponent* VFXComponent);
    void DrawVFXStats();
    void DrawVFXMemoryInfo();
    void DrawVFXLODInfo();
};