#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Engine/Engine.h"
#include "PerformanceProfilerV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceProfiler, Log, All);

DECLARE_STATS_GROUP(TEXT("PerformanceProfiler"), STATGROUP_PerformanceProfiler, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Profiler Update"), STAT_ProfilerUpdate, STATGROUP_PerformanceProfiler);
DECLARE_CYCLE_STAT(TEXT("Data Collection"), STAT_DataCollection, STATGROUP_PerformanceProfiler);
DECLARE_CYCLE_STAT(TEXT("Report Generation"), STAT_ReportGeneration, STATGROUP_PerformanceProfiler);

UENUM(BlueprintType)
enum class EProfilerMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Basic           UMETA(DisplayName = "Basic Monitoring"),
    Detailed        UMETA(DisplayName = "Detailed Analysis"),
    Comprehensive   UMETA(DisplayName = "Comprehensive Profiling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FFramePerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    int32 VisiblePrimitives;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    int32 OccludedPrimitives;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    int32 ActiveDinosaurs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float AIProcessingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    float AudioTime;

    FFramePerformanceData()
    {
        Timestamp = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        VisiblePrimitives = 0;
        OccludedPrimitives = 0;
        ActiveDinosaurs = 0;
        AIProcessingTime = 0.0f;
        PhysicsTime = 0.0f;
        AudioTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    FString SessionName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    FDateTime EndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float SessionDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    int32 TotalFrames;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float MinFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float PercentageAbove60FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float PercentageAbove30FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float AverageMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    float PeakMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    int32 AverageDrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    int32 PeakDrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    int32 HitchCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Report")
    TArray<float> HitchTimes;

    FPerformanceReport()
    {
        SessionName = TEXT("Unknown");
        StartTime = FDateTime::Now();
        EndTime = FDateTime::Now();
        SessionDuration = 0.0f;
        TotalFrames = 0;
        AverageFrameTime = 0.0f;
        MinFrameTime = 0.0f;
        MaxFrameTime = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        PercentageAbove60FPS = 0.0f;
        PercentageAbove30FPS = 0.0f;
        AverageMemoryUsageMB = 0.0f;
        PeakMemoryUsageMB = 0.0f;
        AverageDrawCalls = 0;
        PeakDrawCalls = 0;
        HitchCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float HitchThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MemoryWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 DrawCallWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float GPUTimeWarningThreshold;

    FPerformanceThresholds()
    {
        TargetFrameTime = 16.67f; // 60fps
        HitchThreshold = 50.0f;   // 50ms hitch
        MemoryWarningThreshold = 6144.0f; // 6GB
        DrawCallWarningThreshold = 8000;
        GPUTimeWarningThreshold = 20.0f; // 20ms
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceProfilerV43 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceProfilerV43();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Profiling Control
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void StartProfiling(const FString& SessionName = TEXT("DefaultSession"));

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void PauseProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ResumeProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SetProfilerMode(EProfilerMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    EProfilerMode GetProfilerMode() const { return CurrentMode; }

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SetSamplingRate(float SamplesPerSecond);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SetPerformanceThresholds(const FPerformanceThresholds& Thresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    FPerformanceThresholds GetPerformanceThresholds() const { return Thresholds; }

    // Data Access
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    FFramePerformanceData GetCurrentFrameData();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    TArray<FFramePerformanceData> GetRecentFrameData(int32 FrameCount = 60);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    FPerformanceReport GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SaveReportToFile(const FPerformanceReport& Report, const FString& Filename = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    bool LoadReportFromFile(const FString& Filename, FPerformanceReport& OutReport);

    // Real-time Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void EnableRealTimeAlerts(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void SetAlertCallback(const FString& CallbackName);

    // Specialized Profiling
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ProfileDinosaurAI(int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ProfilePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ProfileRenderingPipeline();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ProfileMemoryUsage();

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    float GetAverageFrameTime(int32 FrameCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    float GetAverageFPS(int32 FrameCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    int32 GetHitchCount(float TimeWindow = 60.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ClearProfileData();

    // Export/Import
    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ExportDataToCSV(const FString& Filename = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Performance Profiler")
    void ExportDataToJSON(const FString& Filename = TEXT(""));

protected:
    // Internal profiling functions
    void UpdateProfileData();
    void CollectFrameData();
    void CheckPerformanceAlerts(const FFramePerformanceData& Data);
    void ProcessHitchDetection(float FrameTime);
    
    // Data collection helpers
    float GetGameThreadTime() const;
    float GetRenderThreadTime() const;
    float GetGPUTime() const;
    int32 GetDrawCallCount() const;
    int32 GetTriangleCount() const;
    float GetMemoryUsage() const;
    int32 GetVisiblePrimitives() const;
    int32 GetOccludedPrimitives() const;
    int32 GetActiveDinosaurCount() const;
    float GetAIProcessingTime() const;
    float GetPhysicsTime() const;
    float GetAudioTime() const;

    // File I/O helpers
    FString GenerateReportFilename(const FString& BaseName = TEXT("PerformanceReport")) const;
    FString GetProfileDataDirectory() const;
    bool EnsureDirectoryExists(const FString& Directory) const;

private:
    UPROPERTY()
    EProfilerMode CurrentMode;

    UPROPERTY()
    FPerformanceThresholds Thresholds;

    // Profiling state
    bool bIsProfilingActive;
    bool bIsPaused;
    bool bRealTimeAlertsEnabled;
    FString CurrentSessionName;
    FDateTime SessionStartTime;
    float SamplingRate;
    float TimeSinceLastSample;

    // Performance data storage
    TArray<FFramePerformanceData> FrameDataHistory;
    int32 MaxHistorySize;
    int32 CurrentDataIndex;

    // Statistics tracking
    float TotalFrameTime;
    int32 TotalFrameCount;
    int32 FramesAbove60FPS;
    int32 FramesAbove30FPS;
    TArray<float> RecentHitches;

    // Timers
    FTimerHandle ProfileUpdateTimer;

    // Alert system
    FString AlertCallbackName;
    float LastAlertTime;
    float AlertCooldownTime;

    // Specialized profiling data
    TMap<FString, float> CustomProfileTimes;
    TArray<float> DinosaurAITimes;
    TArray<float> PhysicsTimes;
    TArray<float> RenderingTimes;
    TArray<float> MemorySnapshots;
};