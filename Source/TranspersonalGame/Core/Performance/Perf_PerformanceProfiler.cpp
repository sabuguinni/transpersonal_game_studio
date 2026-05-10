#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    bIsProfilingActive = false;
    bIsPaused = false;
    MaxSampleCount = 10000;
    ProfilingDuration = 60.0f;
    bAutoExportReports = false;
    ReportExportPath = TEXT("Saved/Profiling/");
}

void UPerf_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeCategorySettings();
    ProfileSamples.Reserve(MaxSampleCount);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized with max samples: %d"), MaxSampleCount);
}

void UPerf_PerformanceProfiler::Deinitialize()
{
    if (bIsProfilingActive)
    {
        StopProfiling();
    }
    
    ProfileSamples.Empty();
    ActiveSamples.Empty();
    
    Super::Deinitialize();
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfilingActive = true;
    bIsPaused = false;
    ProfilingStartTime = FPlatformTime::Seconds();
    LastReportTime = ProfilingStartTime;
    
    ClearSamples();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Performance Profiling Started"));
    }
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling not active"));
        return;
    }
    
    bIsProfilingActive = false;
    bIsPaused = false;
    
    // Generate final report
    FPerf_ProfilerReport FinalReport = GenerateReport();
    LogProfilingResults();
    
    if (bAutoExportReports)
    {
        FString ReportPath = FString::Printf(TEXT("%sProfileReport_%s.txt"), 
            *ReportExportPath, 
            *FDateTime::Now().ToString());
        ExportReportToFile(ReportPath);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped. Total samples: %d"), ProfileSamples.Num());
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
            FString::Printf(TEXT("Profiling Stopped - %d samples collected"), ProfileSamples.Num()));
    }
}

void UPerf_PerformanceProfiler::PauseProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot pause - profiling not active"));
        return;
    }
    
    bIsPaused = true;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling paused"));
}

void UPerf_PerformanceProfiler::ResumeProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot resume - profiling not active"));
        return;
    }
    
    bIsPaused = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling resumed"));
}

void UPerf_PerformanceProfiler::RecordSample(const FString& SampleName, float ExecutionTime, EPerf_ProfilerCategory Category)
{
    if (!bIsProfilingActive || bIsPaused)
    {
        return;
    }
    
    // Check if category is enabled
    if (CategoryEnabled.Contains(Category) && !CategoryEnabled[Category])
    {
        return;
    }
    
    // Check profiling duration
    float CurrentTime = FPlatformTime::Seconds();
    if (ProfilingDuration > 0.0f && (CurrentTime - ProfilingStartTime) > ProfilingDuration)
    {
        StopProfiling();
        return;
    }
    
    // Create new sample
    FPerf_ProfilerSample NewSample(SampleName, ExecutionTime, Category);
    NewSample.Timestamp = CurrentTime;
    
    // Add to samples array
    ProfileSamples.Add(NewSample);
    
    // Clean up if we exceed max samples
    if (ProfileSamples.Num() > MaxSampleCount)
    {
        CleanupOldSamples();
    }
    
    // Log significant performance issues
    if (ExecutionTime > 16.67f) // More than one frame at 60fps
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance spike detected: %s took %.2fms"), *SampleName, ExecutionTime);
    }
}

void UPerf_PerformanceProfiler::BeginSample(const FString& SampleName, EPerf_ProfilerCategory Category)
{
    if (!bIsProfilingActive || bIsPaused)
    {
        return;
    }
    
    // Check if category is enabled
    if (CategoryEnabled.Contains(Category) && !CategoryEnabled[Category])
    {
        return;
    }
    
    double CurrentTime = FPlatformTime::Seconds();
    ActiveSamples.Add(SampleName, CurrentTime);
}

void UPerf_PerformanceProfiler::EndSample(const FString& SampleName)
{
    if (!bIsProfilingActive || bIsPaused)
    {
        return;
    }
    
    double* StartTime = ActiveSamples.Find(SampleName);
    if (!StartTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("EndSample called for unknown sample: %s"), *SampleName);
        return;
    }
    
    double CurrentTime = FPlatformTime::Seconds();
    float ExecutionTime = (CurrentTime - *StartTime) * 1000.0f; // Convert to milliseconds
    
    // Determine category based on sample name (simple heuristic)
    EPerf_ProfilerCategory Category = EPerf_ProfilerCategory::Gameplay;
    if (SampleName.Contains(TEXT("Render")) || SampleName.Contains(TEXT("Draw")))
    {
        Category = EPerf_ProfilerCategory::Rendering;
    }
    else if (SampleName.Contains(TEXT("Physics")) || SampleName.Contains(TEXT("Collision")))
    {
        Category = EPerf_ProfilerCategory::Physics;
    }
    else if (SampleName.Contains(TEXT("AI")) || SampleName.Contains(TEXT("Behavior")))
    {
        Category = EPerf_ProfilerCategory::AI;
    }
    else if (SampleName.Contains(TEXT("Anim")))
    {
        Category = EPerf_ProfilerCategory::Animation;
    }
    else if (SampleName.Contains(TEXT("Audio")) || SampleName.Contains(TEXT("Sound")))
    {
        Category = EPerf_ProfilerCategory::Audio;
    }
    
    RecordSample(SampleName, ExecutionTime, Category);
    ActiveSamples.Remove(SampleName);
}

FPerf_ProfilerReport UPerf_PerformanceProfiler::GenerateReport()
{
    FPerf_ProfilerReport Report;
    
    if (ProfileSamples.Num() == 0)
    {
        return Report;
    }
    
    Report.Samples = ProfileSamples;
    Report.TotalSamples = ProfileSamples.Num();
    
    // Calculate total frame time and average FPS
    float TotalTime = 0.0f;
    for (const FPerf_ProfilerSample& Sample : ProfileSamples)
    {
        TotalTime += Sample.ExecutionTimeMS;
    }
    
    Report.TotalFrameTime = TotalTime;
    Report.ReportDuration = FPlatformTime::Seconds() - ProfilingStartTime;
    
    if (Report.ReportDuration > 0.0f)
    {
        Report.AverageFPS = ProfileSamples.Num() / Report.ReportDuration;
    }
    
    return Report;
}

void UPerf_PerformanceProfiler::ClearSamples()
{
    ProfileSamples.Empty();
    ActiveSamples.Empty();
    ProfilingStartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiler samples cleared"));
}

void UPerf_PerformanceProfiler::ExportReportToFile(const FString& FilePath)
{
    FPerf_ProfilerReport Report = GenerateReport();
    FString ReportString = GenerateReportString(Report);
    
    // Ensure directory exists
    FString Directory = FPaths::GetPath(FilePath);
    IFileManager::Get().MakeDirectory(*Directory, true);
    
    // Write to file
    if (FFileHelper::SaveStringToFile(ReportString, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Performance report exported to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export performance report to: %s"), *FilePath);
    }
}

TArray<FPerf_ProfilerSample> UPerf_PerformanceProfiler::GetSamplesByCategory(EPerf_ProfilerCategory Category)
{
    TArray<FPerf_ProfilerSample> FilteredSamples;
    
    for (const FPerf_ProfilerSample& Sample : ProfileSamples)
    {
        if (Sample.Category == Category)
        {
            FilteredSamples.Add(Sample);
        }
    }
    
    return FilteredSamples;
}

float UPerf_PerformanceProfiler::GetAverageExecutionTime(const FString& SampleName)
{
    TArray<float> ExecutionTimes;
    
    for (const FPerf_ProfilerSample& Sample : ProfileSamples)
    {
        if (Sample.SampleName == SampleName)
        {
            ExecutionTimes.Add(Sample.ExecutionTimeMS);
        }
    }
    
    if (ExecutionTimes.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float Time : ExecutionTimes)
    {
        Total += Time;
    }
    
    return Total / ExecutionTimes.Num();
}

FPerf_ProfilerSample UPerf_PerformanceProfiler::GetWorstPerformingSample()
{
    FPerf_ProfilerSample WorstSample;
    float WorstTime = 0.0f;
    
    for (const FPerf_ProfilerSample& Sample : ProfileSamples)
    {
        if (Sample.ExecutionTimeMS > WorstTime)
        {
            WorstTime = Sample.ExecutionTimeMS;
            WorstSample = Sample;
        }
    }
    
    return WorstSample;
}

TArray<FPerf_ProfilerSample> UPerf_PerformanceProfiler::GetTopWorstSamples(int32 Count)
{
    TArray<FPerf_ProfilerSample> SortedSamples = ProfileSamples;
    
    // Sort by execution time (descending)
    SortedSamples.Sort([](const FPerf_ProfilerSample& A, const FPerf_ProfilerSample& B)
    {
        return A.ExecutionTimeMS > B.ExecutionTimeMS;
    });
    
    // Return top N samples
    TArray<FPerf_ProfilerSample> TopSamples;
    int32 NumToReturn = FMath::Min(Count, SortedSamples.Num());
    
    for (int32 i = 0; i < NumToReturn; i++)
    {
        TopSamples.Add(SortedSamples[i]);
    }
    
    return TopSamples;
}

void UPerf_PerformanceProfiler::SetMaxSampleCount(int32 MaxCount)
{
    MaxSampleCount = FMath::Max(100, MaxCount);
    ProfileSamples.Reserve(MaxSampleCount);
    
    if (ProfileSamples.Num() > MaxSampleCount)
    {
        CleanupOldSamples();
    }
}

void UPerf_PerformanceProfiler::SetProfilingDuration(float Duration)
{
    ProfilingDuration = FMath::Max(0.0f, Duration);
}

void UPerf_PerformanceProfiler::EnableCategoryProfiling(EPerf_ProfilerCategory Category, bool bEnabled)
{
    CategoryEnabled.Add(Category, bEnabled);
}

void UPerf_PerformanceProfiler::InitializeCategorySettings()
{
    // Enable all categories by default
    CategoryEnabled.Add(EPerf_ProfilerCategory::Rendering, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Physics, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::AI, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Animation, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Audio, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Gameplay, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Memory, true);
    CategoryEnabled.Add(EPerf_ProfilerCategory::Network, true);
}

void UPerf_PerformanceProfiler::CleanupOldSamples()
{
    if (ProfileSamples.Num() <= MaxSampleCount)
    {
        return;
    }
    
    // Remove oldest samples (first 25% of array)
    int32 SamplesToRemove = ProfileSamples.Num() / 4;
    ProfileSamples.RemoveAt(0, SamplesToRemove);
}

FString UPerf_PerformanceProfiler::GenerateReportString(const FPerf_ProfilerReport& Report)
{
    FString ReportString;
    
    ReportString += FString::Printf(TEXT("=== PERFORMANCE PROFILING REPORT ===\\n"));
    ReportString += FString::Printf(TEXT("Generated: %s\\n"), *FDateTime::Now().ToString());
    ReportString += FString::Printf(TEXT("Total Samples: %d\\n"), Report.TotalSamples);
    ReportString += FString::Printf(TEXT("Report Duration: %.2f seconds\\n"), Report.ReportDuration);
    ReportString += FString::Printf(TEXT("Average FPS: %.2f\\n"), Report.AverageFPS);
    ReportString += FString::Printf(TEXT("Total Frame Time: %.2f ms\\n\\n"), Report.TotalFrameTime);
    
    // Top worst performing samples
    TArray<FPerf_ProfilerSample> WorstSamples = GetTopWorstSamples(10);
    ReportString += TEXT("=== TOP 10 WORST PERFORMING SAMPLES ===\\n");
    
    for (int32 i = 0; i < WorstSamples.Num(); i++)
    {
        const FPerf_ProfilerSample& Sample = WorstSamples[i];
        ReportString += FString::Printf(TEXT("%d. %s: %.2f ms [%s]\\n"), 
            i + 1, 
            *Sample.SampleName, 
            Sample.ExecutionTimeMS,
            *UEnum::GetValueAsString(Sample.Category));
    }
    
    return ReportString;
}

void UPerf_PerformanceProfiler::LogProfilingResults()
{
    FPerf_ProfilerReport Report = GenerateReport();
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE PROFILING RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Samples: %d"), Report.TotalSamples);
    UE_LOG(LogTemp, Log, TEXT("Average FPS: %.2f"), Report.AverageFPS);
    UE_LOG(LogTemp, Log, TEXT("Report Duration: %.2f seconds"), Report.ReportDuration);
    
    FPerf_ProfilerSample WorstSample = GetWorstPerformingSample();
    if (!WorstSample.SampleName.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Worst Sample: %s (%.2f ms)"), *WorstSample.SampleName, WorstSample.ExecutionTimeMS);
    }
}