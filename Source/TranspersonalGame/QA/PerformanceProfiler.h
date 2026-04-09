#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "PerformanceProfiler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalProfiler, Log, All);

/**
 * Performance Warning Types
 */
UENUM(BlueprintType)
enum class EPerformanceWarningType : uint8
{
    LowFrameRate        UMETA(DisplayName = "Low Frame Rate"),
    HighMemoryUsage     UMETA(DisplayName = "High Memory Usage"),
    HighGPUUsage        UMETA(DisplayName = "High GPU Usage"),
    LongLoadTime        UMETA(DisplayName = "Long Load Time"),
    HighDrawCalls       UMETA(DisplayName = "High Draw Calls"),
    HighTriangleCount   UMETA(DisplayName = "High Triangle Count"),
    ShaderCompilation   UMETA(DisplayName = "Shader Compilation"),
    GarbageCollection   UMETA(DisplayName = "Garbage Collection"),
    NetworkLatency      UMETA(DisplayName = "Network Latency"),
    DiskIO              UMETA(DisplayName = "Disk I/O")
};

/**
 * Performance Warning Severity Levels
 */
UENUM(BlueprintType)
enum class EPerformanceWarningSeverity : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

/**
 * Performance Warning Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceWarning
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Warning")
    EPerformanceWarningType WarningType = EPerformanceWarningType::LowFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Warning")
    EPerformanceWarningSeverity Severity = EPerformanceWarningSeverity::Low;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Warning")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Warning")
    FDateTime Timestamp;

    FPerformanceWarning()
    {
        Timestamp = FDateTime::Now();
    }
};

/**
 * Performance Metrics Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MinFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MaxFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PeakMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentGPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AverageGPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 TotalFramesCounted = 0;
};

/**
 * Delegate declarations for performance events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfilingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfilingStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, const FPerformanceWarning&, Warning);

/**
 * Comprehensive Performance Profiler for Transpersonal Game
 * 
 * Features:
 * - Real-time frame rate monitoring
 * - Memory usage tracking and leak detection
 * - GPU performance monitoring
 * - Automated threshold checking and warnings
 * - Performance report generation
 * - Historical data tracking
 * - Blueprint integration for runtime monitoring
 * 
 * Usage:
 * - Add to any Actor for performance monitoring
 * - Configure thresholds via Blueprint or C++
 * - Start/stop profiling programmatically
 * - Generate reports for QA analysis
 * - Integrate with CI/CD for automated performance validation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranspersonalPerformanceProfiler();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * Start performance profiling
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void StartProfiling();

    /**
     * Stop performance profiling and generate report
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void StopProfiling();

    /**
     * Get current FPS
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    float GetCurrentFPS() const;

    /**
     * Get average frame time in milliseconds
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    float GetAverageFrameTime() const;

    /**
     * Get current memory usage in MB
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    float GetCurrentMemoryUsage() const;

    /**
     * Get peak memory usage in MB
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    float GetPeakMemoryUsage() const;

    /**
     * Get frame time history for graphing
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    TArray<float> GetFrameTimeHistory() const;

    /**
     * Get memory usage history for graphing
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Metrics")
    TArray<float> GetMemoryUsageHistory() const;

    /**
     * Check if profiling is currently active
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiling")
    bool IsProfilingActive() const;

    /**
     * Get the last generated performance report
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Profiling")
    FString GetLastReport() const;

    /**
     * Performance event delegates
     */
    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnProfilingStarted OnProfilingStarted;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnProfilingStopped OnProfilingStopped;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceWarning OnPerformanceWarning;

protected:
    /**
     * Configuration Properties
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Configuration")
    bool bAutoStartProfiling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Configuration")
    bool bLogToFile = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB = 8192.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxGPUMemoryUsageMB = 4096.0f;

    /**
     * Runtime Data
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    bool bIsProfilingActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> FrameTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> MemoryUsageHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> GPUTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<FPerformanceWarning> PerformanceWarnings;

    /**
     * Performance Statistics
     */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float CurrentMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float PeakMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float CurrentGPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    float AverageGPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Statistics")
    int32 TotalFramesCounted;

    /**
     * Internal tracking
     */
    double ProfileStartTime;
    FString LastGeneratedReport;

    /**
     * Internal methods
     */
    void UpdatePerformanceMetrics(float DeltaTime);
    void UpdateMemoryMetrics();
    void UpdateGPUMetrics();
    void CheckPerformanceThresholds();
    void GeneratePerformanceReport(double ProfileDuration);
    void SaveReportToFile(const FString& ReportContent);

public:
    /**
     * Static utility functions for QA testing
     */
    
    /**
     * Force garbage collection and measure impact
     */
    UFUNCTION(BlueprintCallable, Category = "QA Utilities", CallInEditor = true)
    static void ForceGarbageCollectionTest()
    {
        if (GEngine)
        {
            double StartTime = FPlatformTime::Seconds();
            GEngine->ForceGarbageCollection(true);
            double EndTime = FPlatformTime::Seconds();
            double GCTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds
            
            UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Forced GC completed in %.2f ms"), GCTime);
        }
    }

    /**
     * Get current system memory statistics
     */
    UFUNCTION(BlueprintCallable, Category = "QA Utilities", BlueprintPure)
    static FString GetSystemMemoryInfo()
    {
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        
        return FString::Printf(TEXT("Total: %.2f GB, Available: %.2f GB, Used: %.2f GB"),
            MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f),
            MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f),
            MemStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f));
    }

    /**
     * Get current platform information
     */
    UFUNCTION(BlueprintCallable, Category = "QA Utilities", BlueprintPure)
    static FString GetPlatformInfo()
    {
        return FString::Printf(TEXT("Platform: %s, CPU Cores: %d, Logical Cores: %d"),
            *FPlatformProperties::PlatformName(),
            FPlatformMisc::NumberOfCores(),
            FPlatformMisc::NumberOfCoresIncludingHyperthreads());
    }

    /**
     * Stress test memory allocation
     */
    UFUNCTION(BlueprintCallable, Category = "QA Utilities")
    static void RunMemoryStressTest(int32 AllocationSizeMB = 100, int32 NumAllocations = 10)
    {
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Starting memory stress test: %d allocations of %d MB each"), 
            NumAllocations, AllocationSizeMB);
        
        TArray<void*> Allocations;
        size_t AllocationSize = AllocationSizeMB * 1024 * 1024;
        
        for (int32 i = 0; i < NumAllocations; i++)
        {
            void* Memory = FMemory::Malloc(AllocationSize);
            if (Memory)
            {
                Allocations.Add(Memory);
                FMemory::Memset(Memory, 0xFF, AllocationSize); // Touch the memory
                UE_LOG(LogTranspersonalProfiler, Log, TEXT("Allocated block %d"), i + 1);
            }
            else
            {
                UE_LOG(LogTranspersonalProfiler, Error, TEXT("Failed to allocate block %d"), i + 1);
                break;
            }
        }
        
        // Clean up allocations
        for (void* Memory : Allocations)
        {
            FMemory::Free(Memory);
        }
        
        UE_LOG(LogTranspersonalProfiler, Warning, TEXT("Memory stress test completed"));
    }
};