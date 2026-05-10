#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Perf_PerformanceProfiler.generated.h"

UENUM(BlueprintType)
enum class EPerf_ProfilerCategory : uint8
{
    Rendering       UMETA(DisplayName = "Rendering"),
    Physics         UMETA(DisplayName = "Physics"),
    AI              UMETA(DisplayName = "AI"),
    Animation       UMETA(DisplayName = "Animation"),
    Audio           UMETA(DisplayName = "Audio"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Memory          UMETA(DisplayName = "Memory"),
    Network         UMETA(DisplayName = "Network")
};

USTRUCT(BlueprintType)
struct FPerf_ProfilerSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    FString SampleName;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float ExecutionTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    EPerf_ProfilerCategory Category = EPerf_ProfilerCategory::Gameplay;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    int32 CallCount = 0;

    FPerf_ProfilerSample()
    {
        SampleName = TEXT("Unknown");
        ExecutionTimeMS = 0.0f;
        Category = EPerf_ProfilerCategory::Gameplay;
        Timestamp = 0.0f;
        CallCount = 0;
    }

    FPerf_ProfilerSample(const FString& InName, float InTime, EPerf_ProfilerCategory InCategory)
    {
        SampleName = InName;
        ExecutionTimeMS = InTime;
        Category = InCategory;
        Timestamp = FPlatformTime::Seconds();
        CallCount = 1;
    }
};

USTRUCT(BlueprintType)
struct FPerf_ProfilerReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    TArray<FPerf_ProfilerSample> Samples;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float TotalFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    int32 TotalSamples = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float ReportDuration = 0.0f;

    FPerf_ProfilerReport()
    {
        TotalFrameTime = 0.0f;
        AverageFPS = 60.0f;
        TotalSamples = 0;
        ReportDuration = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Profiling controls
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void PauseProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void ResumeProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // Sample recording
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void RecordSample(const FString& SampleName, float ExecutionTime, EPerf_ProfilerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void BeginSample(const FString& SampleName, EPerf_ProfilerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void EndSample(const FString& SampleName);

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    FPerf_ProfilerReport GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void ClearSamples();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void ExportReportToFile(const FString& FilePath);

    // Analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    TArray<FPerf_ProfilerSample> GetSamplesByCategory(EPerf_ProfilerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    float GetAverageExecutionTime(const FString& SampleName);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    FPerf_ProfilerSample GetWorstPerformingSample();

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    TArray<FPerf_ProfilerSample> GetTopWorstSamples(int32 Count = 10);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void SetMaxSampleCount(int32 MaxCount);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void SetProfilingDuration(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Performance Profiling")
    void EnableCategoryProfiling(EPerf_ProfilerCategory Category, bool bEnabled);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bIsProfilingActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bIsPaused = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxSampleCount = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ProfilingDuration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoExportReports = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString ReportExportPath = TEXT("Saved/Profiling/");

private:
    TArray<FPerf_ProfilerSample> ProfileSamples;
    TMap<FString, double> ActiveSamples;
    TMap<EPerf_ProfilerCategory, bool> CategoryEnabled;
    
    float ProfilingStartTime = 0.0f;
    float LastReportTime = 0.0f;
    
    void InitializeCategorySettings();
    void CleanupOldSamples();
    FString GenerateReportString(const FPerf_ProfilerReport& Report);
    void LogProfilingResults();
};

// Helper macros for easy profiling
#define PERF_PROFILE_SCOPE(Name, Category) \
    UPerf_PerformanceProfiler* Profiler = GetGameInstance()->GetSubsystem<UPerf_PerformanceProfiler>(); \
    if (Profiler && Profiler->IsProfilingActive()) \
    { \
        Profiler->BeginSample(Name, Category); \
    } \
    ON_SCOPE_EXIT \
    { \
        if (Profiler && Profiler->IsProfilingActive()) \
        { \
            Profiler->EndSample(Name); \
        } \
    };

#define PERF_PROFILE_FUNCTION(Category) PERF_PROFILE_SCOPE(__FUNCTION__, Category)