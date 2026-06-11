#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DroppedFrames;

    FPerf_FrameStats()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        MinFrameTime = 0.0f;
        MaxFrameTime = 0.0f;
        DroppedFrames = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedVirtualMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 ActiveObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 GarbageCollectionCount;

    FPerf_MemoryStats()
    {
        UsedPhysicalMemoryMB = 0.0f;
        UsedVirtualMemoryMB = 0.0f;
        AvailablePhysicalMemoryMB = 0.0f;
        ActiveObjects = 0;
        GarbageCollectionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 VisiblePrimitives;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    float GPUFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    float RenderThreadTime;

    FPerf_RenderStats()
    {
        DrawCalls = 0;
        Triangles = 0;
        VisiblePrimitives = 0;
        GPUFrameTime = 0.0f;
        RenderThreadTime = 0.0f;
    }
};

/**
 * Performance profiler component that monitors game performance metrics
 * Tracks FPS, memory usage, rendering stats, and provides optimization recommendations
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStats();

    // Getters for performance data
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameStats GetFrameStats() const { return FrameStats; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_MemoryStats GetMemoryStats() const { return MemoryStats; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_RenderStats GetRenderStats() const { return RenderStats; }

    // Performance level assessment
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FString> GetOptimizationRecommendations() const;

    // Performance targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float InTargetFPS) { TargetFPS = InTargetFPS; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetTargetFPS() const { return TargetFPS; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceTargetMet() const;

protected:
    // Update functions
    void UpdateFrameStats(float DeltaTime);
    void UpdateMemoryStats();
    void UpdateRenderStats();

    // Analysis functions
    void AnalyzePerformance();
    void GenerateRecommendations();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfilingActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_FrameStats FrameStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_MemoryStats MemoryStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_RenderStats RenderStats;

    // Internal tracking
    TArray<float> FrameTimeHistory;
    float ProfilingStartTime;
    float LastUpdateTime;
    int32 FrameCount;
    
    // Performance thresholds
    static constexpr float LOW_PERFORMANCE_THRESHOLD = 30.0f;
    static constexpr float MEDIUM_PERFORMANCE_THRESHOLD = 45.0f;
    static constexpr float HIGH_PERFORMANCE_THRESHOLD = 60.0f;
    static constexpr int32 FRAME_HISTORY_SIZE = 120; // 2 seconds at 60fps
};