#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Core_PhysicsProfiler.generated.h"

/**
 * Physics Performance Profiler Component
 * Monitors and profiles physics performance in real-time
 * Provides detailed metrics for optimization decisions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    int32 SleepingRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    int32 CollisionPairs;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiler")
    float MemoryUsageMB;

    // Profiling Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    bool bEnableProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    bool bLogToConsole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    bool bDisplayOnScreen;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    float WarningFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    float CriticalFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profiler")
    int32 MaxActiveRigidBodies;

    // Profiling Methods
    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    FString GetPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    bool IsPerformanceWithinLimits();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void LogCurrentMetrics();

    // Advanced Profiling
    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void ProfilePhysicsStep();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void ProfileCollisionDetection();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void ProfileConstraintSolving();

    // Optimization Suggestions
    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    TArray<FString> GetOptimizationSuggestions();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiler")
    void ApplyAutoOptimizations();

private:
    // Internal Profiling Data
    float LastProfilingTime;
    TArray<float> FrameTimeHistory;
    TArray<int32> RigidBodyCountHistory;
    
    // Performance Analysis
    void UpdateMetrics();
    void AnalyzePerformance();
    void DisplayMetricsOnScreen();
    
    // Optimization Logic
    void SuggestLODOptimizations();
    void SuggestCullingOptimizations();
    void SuggestPhysicsSettingsOptimizations();
};