/**
 * @file ConsciousnessPhysicsOptimized.h
 * @brief Performance-optimized consciousness physics system
 * @author Performance Optimizer
 * @version 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "HAL/ThreadSafeBool.h"
#include "Async/TaskGraphInterfaces.h"
#include "ConsciousnessPhysics.h"
#include "ConsciousnessPhysicsOptimized.generated.h"

// Forward declarations
class UConsciousnessFieldComponent;
class FConsciousnessPhysicsTask;

/**
 * Performance optimization levels for consciousness physics
 */
UENUM(BlueprintType)
enum class EConsciousnessOptimizationLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Balanced"),
    Low         UMETA(DisplayName = "Performance"),
    Adaptive    UMETA(DisplayName = "Adaptive Quality")
};

/**
 * Optimized consciousness physics component with performance monitoring
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessPhysicsOptimized : public UConsciousnessPhysicsComponent
{
    GENERATED_BODY()

public:
    UConsciousnessPhysicsOptimized();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Performance optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EConsciousnessOptimizationLevel OptimizationLevel = EConsciousnessOptimizationLevel::Adaptive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.016", ClampMax = "0.1"))
    float TargetFrameTime = 0.016f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "1000"))
    int32 MaxFieldUpdatesPerFrame = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float LODDistance = 2000.0f;

    // Threading settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading")
    bool bUseMultithreading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threading", meta = (ClampMin = "1", ClampMax = "16"))
    int32 MaxWorkerThreads = 4;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    int32 ActiveFieldCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Monitoring")
    float AverageUpdateTime = 0.0f;

    // Optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EConsciousnessOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentFramerate();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetPerformanceScore() const;

private:
    // Performance tracking
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 60;
    int32 FrameHistoryIndex = 0;

    // Threading
    FThreadSafeBool bThreadingEnabled;
    TArray<TSharedPtr<FConsciousnessPhysicsTask>> WorkerTasks;
    FCriticalSection TaskMutex;

    // LOD system
    TMap<UConsciousnessFieldComponent*, float> FieldDistances;
    float LastLODUpdateTime = 0.0f;
    static constexpr float LODUpdateInterval = 0.1f; // Update LOD every 100ms

    // Adaptive quality
    bool bAdaptiveQualityEnabled = true;
    float QualityAdjustmentCooldown = 0.0f;
    static constexpr float QualityAdjustmentInterval = 1.0f;

    // Internal optimization methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void UpdateLODSystem();
    void AdjustQualityBasedOnPerformance();
    void OptimizeFieldUpdates();
    void InitializeThreading();
    void ShutdownThreading();
    
    // Utility functions
    float CalculateDistanceToPlayer(const FVector& Position) const;
    bool ShouldUpdateField(UConsciousnessFieldComponent* Field, float Distance) const;
    void UpdateFieldWithLOD(UConsciousnessFieldComponent* Field, float Distance, float DeltaTime);
};

/**
 * Async task for consciousness physics calculations
 */
class FConsciousnessPhysicsTask : public FNonAbandonableTask
{
public:
    FConsciousnessPhysicsTask(TWeakObjectPtr<UConsciousnessPhysicsOptimized> InComponent, 
                             TArray<TWeakObjectPtr<UConsciousnessFieldComponent>> InFields,
                             float InDeltaTime);

    void DoWork();
    
    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FConsciousnessPhysicsTask, STATGROUP_ThreadPoolAsyncTasks);
    }

private:
    TWeakObjectPtr<UConsciousnessPhysicsOptimized> Component;
    TArray<TWeakObjectPtr<UConsciousnessFieldComponent>> Fields;
    float DeltaTime;
};

/**
 * Performance profiler for consciousness physics
 */
UCLASS()
class TRANSPERSONALGAME_API UConsciousnessPhysicsProfiler : public UObject
{
    GENERATED_BODY()

public:
    static UConsciousnessPhysicsProfiler* GetInstance();

    void BeginFrame();
    void EndFrame();
    void RecordFieldUpdate(float UpdateTime);
    void RecordResonanceCalculation(float CalculationTime);

    UFUNCTION(BlueprintPure, Category = "Profiling")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintPure, Category = "Profiling")
    float GetAverageFieldUpdateTime() const { return AverageFieldUpdateTime; }

    UFUNCTION(BlueprintPure, Category = "Profiling")
    int32 GetFieldUpdatesPerFrame() const { return FieldUpdatesThisFrame; }

private:
    static UConsciousnessPhysicsProfiler* Instance;

    // Performance metrics
    float FrameStartTime = 0.0f;
    float AverageFrameTime = 0.0f;
    float AverageFieldUpdateTime = 0.0f;
    int32 FieldUpdatesThisFrame = 0;
    
    // History for averaging
    TArray<float> FrameTimeHistory;
    TArray<float> FieldUpdateTimeHistory;
    
    void UpdateAverages();
};