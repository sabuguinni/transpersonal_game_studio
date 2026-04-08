/**
 * @file PhysicsProfiler.h
 * @brief Advanced profiling system for consciousness physics performance
 * @author Performance Optimizer Agent
 * @version 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/ThreadSafeCounter.h"
#include "Containers/CircularBuffer.h"
#include "Stats/Stats.h"
#include "PhysicsProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsProfiler, Log, All);

/**
 * Performance bottleneck types
 */
UENUM(BlueprintType)
enum class EBottleneckType : uint8
{
    None            UMETA(DisplayName = "No Bottleneck"),
    CPU             UMETA(DisplayName = "CPU Bound"),
    Memory          UMETA(DisplayName = "Memory Bound"),
    Physics         UMETA(DisplayName = "Physics Bound"),
    Rendering       UMETA(DisplayName = "Rendering Bound"),
    Threading       UMETA(DisplayName = "Threading Bound")
};

/**
 * Detailed profiling data for a single frame
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFrameProfileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessCalculationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float RenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MemoryAllocTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsCalculations = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveEntities = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EBottleneckType PrimaryBottleneck = EBottleneckType::None;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;
};

/**
 * Performance trend analysis
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceTrend
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MinFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MaxFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FrameTimeVariance = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TrendSlope = 0.0f; // Positive = getting worse, Negative = getting better

    UPROPERTY(BlueprintReadOnly)
    EBottleneckType MostCommonBottleneck = EBottleneckType::None;
};

/**
 * Profiling recommendations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FProfilerRecommendation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString Title;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority = 0; // 1 = Critical, 2 = High, 3 = Medium, 4 = Low

    UPROPERTY(BlueprintReadOnly)
    float EstimatedImpact = 0.0f; // Percentage improvement expected

    UPROPERTY(BlueprintReadOnly)
    EBottleneckType TargetBottleneck = EBottleneckType::None;
};

/**
 * Advanced physics profiling system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPhysicsProfiler();

    // Core profiling functions
    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void BeginFrame();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void EndFrame();

    UFUNCTION(BlueprintCallable, Category = "Profiling")
    void ProfilePhysicsCalculation(const FString& CalculationName, float ExecutionTime);

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    FPerformanceTrend AnalyzeTrends(int32 FrameCount = 300) const;

    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    TArray<FProfilerRecommendation> GenerateRecommendations() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    EBottleneckType IdentifyBottleneck(const FFrameProfileData& FrameData) const;

    // Data access
    UFUNCTION(BlueprintPure, Category = "Profiling")
    FFrameProfileData GetCurrentFrameData() const { return CurrentFrameData; }

    UFUNCTION(BlueprintPure, Category = "Profiling")
    TArray<FFrameProfileData> GetRecentFrameData(int32 FrameCount = 60) const;

    UFUNCTION(BlueprintPure, Category = "Profiling")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // Export functions
    UFUNCTION(BlueprintCallable, Category = "Profiling|Export")
    void ExportProfilingData(const FString& FilePath) const;

    UFUNCTION(BlueprintCallable, Category = "Profiling|Export")
    FString GeneratePerformanceReport() const;

protected:
    // Profiling state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsProfilingActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FFrameProfileData CurrentFrameData;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxFrameHistory = 1800; // 30 seconds at 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoGenerateRecommendations = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BottleneckThreshold = 16.67f; // 60 FPS threshold in milliseconds

    // Frame history
    UPROPERTY()
    TArray<FFrameProfileData> FrameHistory;

    // Timing data
    UPROPERTY()
    double FrameStartTime = 0.0;

    UPROPERTY()
    double PhysicsStartTime = 0.0;

    UPROPERTY()
    TMap<FString, float> CalculationTimes;

    // Counters
    UPROPERTY()
    FThreadSafeCounter PhysicsCalculationCounter;

    UPROPERTY()
    FThreadSafeCounter ActiveEntityCounter;

private:
    // Internal profiling methods
    void UpdateFrameData();
    void AnalyzeBottlenecks();
    void UpdateTrendData();
    
    // Recommendation generation
    TArray<FProfilerRecommendation> GenerateCPURecommendations() const;
    TArray<FProfilerRecommendation> GenerateMemoryRecommendations() const;
    TArray<FProfilerRecommendation> GeneratePhysicsRecommendations() const;
    
    // Analysis helpers
    float CalculateFrameTimeVariance(const TArray<FFrameProfileData>& Frames) const;
    float CalculateTrendSlope(const TArray<FFrameProfileData>& Frames) const;
    EBottleneckType GetMostCommonBottleneck(const TArray<FFrameProfileData>& Frames) const;
    
    // Utility functions
    FString FormatTime(float TimeMs) const;
    FString FormatMemory(float MemoryMB) const;
    FString GetBottleneckDescription(EBottleneckType Bottleneck) const;
};

/**
 * Scoped profiler for automatic timing
 */
class TRANSPERSONALGAME_API FScopedPhysicsProfiler
{
public:
    FScopedPhysicsProfiler(UPhysicsProfiler* InProfiler, const FString& InName);
    ~FScopedPhysicsProfiler();

private:
    UPhysicsProfiler* Profiler;
    FString Name;
    double StartTime;
};

// Convenience macro for scoped profiling
#define SCOPE_PHYSICS_PROFILER(Profiler, Name) \
    FScopedPhysicsProfiler ScopedProfiler##__LINE__(Profiler, Name)