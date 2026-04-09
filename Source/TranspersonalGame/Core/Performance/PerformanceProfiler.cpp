// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/StatsHierarchical.h"
#include "RenderingThread.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMemory.h"
#include "RHI.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/MiscTrace.h"

DEFINE_LOG_CATEGORY(LogPerformanceProfiler);

// Console commands for profiling
static FAutoConsoleCommand CmdStartProfiling(
    TEXT("tp.StartProfiling"),
    TEXT("Start performance profiling"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        if (UPerformanceProfiler* Profiler = UPerformanceProfiler::GetGlobalProfiler())
        {
            Profiler->StartProfiling();
        }
    })
);

static FAutoConsoleCommand CmdStopProfiling(
    TEXT("tp.StopProfiling"),
    TEXT("Stop performance profiling and save results"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        if (UPerformanceProfiler* Profiler = UPerformanceProfiler::GetGlobalProfiler())
        {
            Profiler->StopProfiling();
        }
    })
);

static FAutoConsoleCommand CmdDumpProfileData(
    TEXT("tp.DumpProfileData"),
    TEXT("Dump current performance profile data to log"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        if (UPerformanceProfiler* Profiler = UPerformanceProfiler::GetGlobalProfiler())
        {
            Profiler->DumpProfileData();
        }
    })
);

UPerformanceProfiler* UPerformanceProfiler::GlobalProfiler = nullptr;

UPerformanceProfiler::UPerformanceProfiler()
{
    bIsProfilingActive = false;
    ProfileStartTime = 0.0;
    ProfileDuration = 0.0;
    SampleInterval = 0.1f; // Sample every 100ms
    MaxSamples = 10000; // Store up to 10000 samples
    
    // Initialize sample arrays
    FrameTimeSamples.Reserve(MaxSamples);
    GameThreadSamples.Reserve(MaxSamples);
    RenderThreadSamples.Reserve(MaxSamples);
    GPUSamples.Reserve(MaxSamples);
    MemorySamples.Reserve(MaxSamples);
    PhysicsSamples.Reserve(MaxSamples);
    AISamples.Reserve(MaxSamples);
    
    GlobalProfiler = this;
}

UPerformanceProfiler::~UPerformanceProfiler()
{
    if (GlobalProfiler == this)
    {
        GlobalProfiler = nullptr;
    }
}

void UPerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance Profiler initialized"));
    
    // Set up profiling timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProfilingTimer,
            this,
            &UPerformanceProfiler::SamplePerformanceData,
            SampleInterval,
            true
        );
    }
}

void UPerformanceProfiler::Deinitialize()
{
    if (bIsProfilingActive)
    {
        StopProfiling();
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProfilingTimer);
    }
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Performance Profiler deinitialized"));
    
    Super::Deinitialize();
}

UPerformanceProfiler* UPerformanceProfiler::GetGlobalProfiler()
{
    return GlobalProfiler;
}

UPerformanceProfiler* UPerformanceProfiler::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPerformanceProfiler>();
    }
    return nullptr;
}

void UPerformanceProfiler::StartProfiling(const FString& ProfileName)
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Profiling already active. Stopping previous session."));
        StopProfiling();
    }
    
    CurrentProfileName = ProfileName.IsEmpty() ? FString::Printf(TEXT("Profile_%s"), *FDateTime::Now().ToString()) : ProfileName;
    
    // Clear previous data
    ClearProfileData();
    
    // Start profiling
    bIsProfilingActive = true;
    ProfileStartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Started performance profiling: %s"), *CurrentProfileName);
    
    // Start Unreal Insights tracing if available
    StartUnrealInsightsTrace();
}

void UPerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("No active profiling session to stop"));
        return;
    }
    
    bIsProfilingActive = false;
    ProfileDuration = FPlatformTime::Seconds() - ProfileStartTime;
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Stopped performance profiling: %s (Duration: %.2fs)"), 
           *CurrentProfileName, ProfileDuration);
    
    // Stop Unreal Insights tracing
    StopUnrealInsightsTrace();
    
    // Save profile data
    SaveProfileData();
    
    // Generate performance report
    GeneratePerformanceReport();
}

void UPerformanceProfiler::StartUnrealInsightsTrace()
{
    // Start Unreal Insights trace with relevant channels
    static IConsoleVariable* CVarTraceChannels = IConsoleManager::Get().FindConsoleVariable(TEXT("trace"));
    
    if (CVarTraceChannels)
    {
        // Enable key performance channels
        FString TraceChannels = TEXT("cpu,gpu,memory,frame,stats");
        CVarTraceChannels->Set(*TraceChannels);
        
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Started Unreal Insights trace with channels: %s"), *TraceChannels);
    }
}

void UPerformanceProfiler::StopUnrealInsightsTrace()
{
    // Stop Unreal Insights trace
    static IConsoleVariable* CVarTraceChannels = IConsoleManager::Get().FindConsoleVariable(TEXT("trace"));
    
    if (CVarTraceChannels)
    {
        CVarTraceChannels->Set(TEXT(""));
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Stopped Unreal Insights trace"));
    }
}

void UPerformanceProfiler::SamplePerformanceData()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    // Check if we've exceeded max samples
    if (FrameTimeSamples.Num() >= MaxSamples)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("Maximum samples reached. Stopping profiling."));
        StopProfiling();
        return;
    }
    
    double CurrentTime = FPlatformTime::Seconds() - ProfileStartTime;
    
    // Sample frame time
    float FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    FrameTimeSamples.Add(FPerformanceSample(CurrentTime, FrameTime));
    
    // Sample game thread time
    float GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    GameThreadSamples.Add(FPerformanceSample(CurrentTime, GameThreadTime));
    
    // Sample render thread time
    float RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    RenderThreadSamples.Add(FPerformanceSample(CurrentTime, RenderThreadTime));
    
    // Sample GPU time
    float GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
    GPUSamples.Add(FPerformanceSample(CurrentTime, GPUTime));
    
    // Sample memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    MemorySamples.Add(FPerformanceSample(CurrentTime, MemoryUsageMB));
    
    // Sample physics performance (placeholder - would need integration with physics system)
    float PhysicsTime = 0.0f; // TODO: Get actual physics time
    PhysicsSamples.Add(FPerformanceSample(CurrentTime, PhysicsTime));
    
    // Sample AI performance (placeholder - would need integration with AI system)
    float AITime = 0.0f; // TODO: Get actual AI time
    AISamples.Add(FPerformanceSample(CurrentTime, AITime));
}

void UPerformanceProfiler::ClearProfileData()
{
    FrameTimeSamples.Empty();
    GameThreadSamples.Empty();
    RenderThreadSamples.Empty();
    GPUSamples.Empty();
    MemorySamples.Empty();
    PhysicsSamples.Empty();
    AISamples.Empty();
    
    ProfileEvents.Empty();
}

void UPerformanceProfiler::SaveProfileData()
{
    FString SaveDirectory = FPaths::ProjectSavedDir() / TEXT("Profiling");
    FString FileName = FString::Printf(TEXT("%s_%s.json"), *CurrentProfileName, *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FString FilePath = SaveDirectory / FileName;
    
    // Ensure directory exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDirectory))
    {
        PlatformFile.CreateDirectoryTree(*SaveDirectory);
    }
    
    // Create JSON data
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    // Profile metadata
    JsonObject->SetStringField(TEXT("ProfileName"), CurrentProfileName);
    JsonObject->SetStringField(TEXT("StartTime"), FDateTime::FromUnixTimestamp(ProfileStartTime).ToString());
    JsonObject->SetNumberField(TEXT("Duration"), ProfileDuration);
    JsonObject->SetNumberField(TEXT("SampleCount"), FrameTimeSamples.Num());
    JsonObject->SetNumberField(TEXT("SampleInterval"), SampleInterval);
    
    // Performance statistics
    TSharedPtr<FJsonObject> StatsObject = MakeShareable(new FJsonObject);
    
    FPerformanceStatistics FrameStats = CalculateStatistics(FrameTimeSamples);
    TSharedPtr<FJsonObject> FrameStatsJson = StatisticsToJson(FrameStats);
    StatsObject->SetObjectField(TEXT("FrameTime"), FrameStatsJson);
    
    FPerformanceStatistics GameThreadStats = CalculateStatistics(GameThreadSamples);
    TSharedPtr<FJsonObject> GameThreadStatsJson = StatisticsToJson(GameThreadStats);
    StatsObject->SetObjectField(TEXT("GameThread"), GameThreadStatsJson);
    
    FPerformanceStatistics RenderThreadStats = CalculateStatistics(RenderThreadSamples);
    TSharedPtr<FJsonObject> RenderThreadStatsJson = StatisticsToJson(RenderThreadStats);
    StatsObject->SetObjectField(TEXT("RenderThread"), RenderThreadStatsJson);
    
    FPerformanceStatistics GPUStats = CalculateStatistics(GPUSamples);
    TSharedPtr<FJsonObject> GPUStatsJson = StatisticsToJson(GPUStats);
    StatsObject->SetObjectField(TEXT("GPU"), GPUStatsJson);
    
    FPerformanceStatistics MemoryStats = CalculateStatistics(MemorySamples);
    TSharedPtr<FJsonObject> MemoryStatsJson = StatisticsToJson(MemoryStats);
    StatsObject->SetObjectField(TEXT("Memory"), MemoryStatsJson);
    
    JsonObject->SetObjectField(TEXT("Statistics"), StatsObject);
    
    // Sample data arrays
    TSharedPtr<FJsonObject> SamplesObject = MakeShareable(new FJsonObject);
    SamplesObject->SetArrayField(TEXT("FrameTime"), SamplesToJsonArray(FrameTimeSamples));
    SamplesObject->SetArrayField(TEXT("GameThread"), SamplesToJsonArray(GameThreadSamples));
    SamplesObject->SetArrayField(TEXT("RenderThread"), SamplesToJsonArray(RenderThreadSamples));
    SamplesObject->SetArrayField(TEXT("GPU"), SamplesToJsonArray(GPUSamples));
    SamplesObject->SetArrayField(TEXT("Memory"), SamplesToJsonArray(MemorySamples));
    SamplesObject->SetArrayField(TEXT("Physics"), SamplesToJsonArray(PhysicsSamples));
    SamplesObject->SetArrayField(TEXT("AI"), SamplesToJsonArray(AISamples));
    
    JsonObject->SetObjectField(TEXT("Samples"), SamplesObject);
    
    // Events
    TArray<TSharedPtr<FJsonValue>> EventsArray;
    for (const FPerformanceEvent& Event : ProfileEvents)
    {
        TSharedPtr<FJsonObject> EventObject = MakeShareable(new FJsonObject);
        EventObject->SetStringField(TEXT("Name"), Event.EventName);
        EventObject->SetStringField(TEXT("Category"), Event.Category);
        EventObject->SetNumberField(TEXT("StartTime"), Event.StartTime);
        EventObject->SetNumberField(TEXT("EndTime"), Event.EndTime);
        EventObject->SetNumberField(TEXT("Duration"), Event.Duration);
        
        EventsArray.Add(MakeShareable(new FJsonValueObject(EventObject)));
    }
    JsonObject->SetArrayField(TEXT("Events"), EventsArray);
    
    // Write JSON to file
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Profile data saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogPerformanceProfiler, Error, TEXT("Failed to save profile data to: %s"), *FilePath);
    }
}

void UPerformanceProfiler::GeneratePerformanceReport()
{
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== PERFORMANCE REPORT: %s ==="), *CurrentProfileName);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Duration: %.2f seconds"), ProfileDuration);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Samples: %d"), FrameTimeSamples.Num());
    
    // Frame time statistics
    FPerformanceStatistics FrameStats = CalculateStatistics(FrameTimeSamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("FRAME TIME (ms):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average: %.2f"), FrameStats.Average);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Min: %.2f"), FrameStats.Min);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Max: %.2f"), FrameStats.Max);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  95th Percentile: %.2f"), FrameStats.Percentile95);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  99th Percentile: %.2f"), FrameStats.Percentile99);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average FPS: %.1f"), 1000.0f / FrameStats.Average);
    
    // Game thread statistics
    FPerformanceStatistics GameThreadStats = CalculateStatistics(GameThreadSamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("GAME THREAD (ms):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average: %.2f"), GameThreadStats.Average);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Min: %.2f"), GameThreadStats.Min);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Max: %.2f"), GameThreadStats.Max);
    
    // Render thread statistics
    FPerformanceStatistics RenderThreadStats = CalculateStatistics(RenderThreadSamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("RENDER THREAD (ms):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average: %.2f"), RenderThreadStats.Average);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Min: %.2f"), RenderThreadStats.Min);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Max: %.2f"), RenderThreadStats.Max);
    
    // GPU statistics
    FPerformanceStatistics GPUStats = CalculateStatistics(GPUSamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("GPU (ms):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average: %.2f"), GPUStats.Average);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Min: %.2f"), GPUStats.Min);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Max: %.2f"), GPUStats.Max);
    
    // Memory statistics
    FPerformanceStatistics MemoryStats = CalculateStatistics(MemorySamples);
    UE_LOG(LogPerformanceProfiler, Log, TEXT(""));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("MEMORY (MB):"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Average: %.1f"), MemoryStats.Average);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Min: %.1f"), MemoryStats.Min);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("  Max: %.1f"), MemoryStats.Max);
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== END PERFORMANCE REPORT ==="));
}

FPerformanceStatistics UPerformanceProfiler::CalculateStatistics(const TArray<FPerformanceSample>& Samples)
{
    FPerformanceStatistics Stats;
    
    if (Samples.Num() == 0)
    {
        return Stats;
    }
    
    // Extract values and sort for percentile calculations
    TArray<float> Values;
    Values.Reserve(Samples.Num());
    
    float Sum = 0.0f;
    Stats.Min = FLT_MAX;
    Stats.Max = -FLT_MAX;
    
    for (const FPerformanceSample& Sample : Samples)
    {
        Values.Add(Sample.Value);
        Sum += Sample.Value;
        Stats.Min = FMath::Min(Stats.Min, Sample.Value);
        Stats.Max = FMath::Max(Stats.Max, Sample.Value);
    }
    
    Stats.Average = Sum / Samples.Num();
    
    // Sort for percentile calculations
    Values.Sort();
    
    // Calculate percentiles
    int32 Index95 = FMath::FloorToInt(Values.Num() * 0.95f);
    int32 Index99 = FMath::FloorToInt(Values.Num() * 0.99f);
    
    Stats.Percentile95 = Values[FMath::Clamp(Index95, 0, Values.Num() - 1)];
    Stats.Percentile99 = Values[FMath::Clamp(Index99, 0, Values.Num() - 1)];
    
    // Calculate standard deviation
    float VarianceSum = 0.0f;
    for (float Value : Values)
    {
        float Diff = Value - Stats.Average;
        VarianceSum += Diff * Diff;
    }
    Stats.StandardDeviation = FMath::Sqrt(VarianceSum / Values.Num());
    
    return Stats;
}

TSharedPtr<FJsonObject> UPerformanceProfiler::StatisticsToJson(const FPerformanceStatistics& Stats)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    JsonObject->SetNumberField(TEXT("Average"), Stats.Average);
    JsonObject->SetNumberField(TEXT("Min"), Stats.Min);
    JsonObject->SetNumberField(TEXT("Max"), Stats.Max);
    JsonObject->SetNumberField(TEXT("StandardDeviation"), Stats.StandardDeviation);
    JsonObject->SetNumberField(TEXT("Percentile95"), Stats.Percentile95);
    JsonObject->SetNumberField(TEXT("Percentile99"), Stats.Percentile99);
    
    return JsonObject;
}

TArray<TSharedPtr<FJsonValue>> UPerformanceProfiler::SamplesToJsonArray(const TArray<FPerformanceSample>& Samples)
{
    TArray<TSharedPtr<FJsonValue>> JsonArray;
    
    for (const FPerformanceSample& Sample : Samples)
    {
        TSharedPtr<FJsonObject> SampleObject = MakeShareable(new FJsonObject);
        SampleObject->SetNumberField(TEXT("Time"), Sample.Timestamp);
        SampleObject->SetNumberField(TEXT("Value"), Sample.Value);
        
        JsonArray.Add(MakeShareable(new FJsonValueObject(SampleObject)));
    }
    
    return JsonArray;
}

void UPerformanceProfiler::AddPerformanceEvent(const FString& EventName, const FString& Category, double StartTime, double EndTime)
{
    FPerformanceEvent Event;
    Event.EventName = EventName;
    Event.Category = Category;
    Event.StartTime = StartTime;
    Event.EndTime = EndTime;
    Event.Duration = EndTime - StartTime;
    
    ProfileEvents.Add(Event);
}

void UPerformanceProfiler::MarkPerformanceEvent(const FString& EventName, const FString& Category)
{
    double CurrentTime = FPlatformTime::Seconds() - ProfileStartTime;
    AddPerformanceEvent(EventName, Category, CurrentTime, CurrentTime);
}

FPerformanceStatistics UPerformanceProfiler::GetFrameTimeStatistics() const
{
    return CalculateStatistics(FrameTimeSamples);
}

FPerformanceStatistics UPerformanceProfiler::GetGameThreadStatistics() const
{
    return CalculateStatistics(GameThreadSamples);
}

FPerformanceStatistics UPerformanceProfiler::GetRenderThreadStatistics() const
{
    return CalculateStatistics(RenderThreadSamples);
}

FPerformanceStatistics UPerformanceProfiler::GetGPUStatistics() const
{
    return CalculateStatistics(GPUSamples);
}

FPerformanceStatistics UPerformanceProfiler::GetMemoryStatistics() const
{
    return CalculateStatistics(MemorySamples);
}

void UPerformanceProfiler::DumpProfileData()
{
    if (!bIsProfilingActive && FrameTimeSamples.Num() == 0)
    {
        UE_LOG(LogPerformanceProfiler, Warning, TEXT("No profile data available"));
        return;
    }
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== CURRENT PROFILE DATA ==="));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Profile: %s"), *CurrentProfileName);
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Active: %s"), bIsProfilingActive ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Samples: %d"), FrameTimeSamples.Num());
    
    if (FrameTimeSamples.Num() > 0)
    {
        FPerformanceStatistics FrameStats = CalculateStatistics(FrameTimeSamples);
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Current Average FPS: %.1f"), 1000.0f / FrameStats.Average);
        UE_LOG(LogPerformanceProfiler, Log, TEXT("Current Frame Time: %.2f ms"), FrameStats.Average);
    }
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("=== END PROFILE DATA ==="));
}

void UPerformanceProfiler::SetSampleInterval(float Interval)
{
    SampleInterval = FMath::Clamp(Interval, 0.01f, 1.0f);
    
    // Update timer if active
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProfilingTimer,
            this,
            &UPerformanceProfiler::SamplePerformanceData,
            SampleInterval,
            true
        );
    }
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Sample interval set to: %.3f seconds"), SampleInterval);
}

void UPerformanceProfiler::SetMaxSamples(int32 MaxSampleCount)
{
    MaxSamples = FMath::Clamp(MaxSampleCount, 100, 100000);
    
    UE_LOG(LogPerformanceProfiler, Log, TEXT("Max samples set to: %d"), MaxSamples);
}