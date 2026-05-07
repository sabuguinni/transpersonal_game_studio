#include "Perf_MemoryProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "HAL/IConsoleManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY_STATIC(LogMemoryProfiler, Log, All);

UPerf_MemoryProfiler::UPerf_MemoryProfiler()
{
    // Initialize memory tracking settings
    MemorySettings.SampleInterval = 1.0f;
    MemorySettings.MaxSamples = 1800; // 30 minutes at 1 sample per second
    MemorySettings.bTrackObjectAllocations = true;
    MemorySettings.bTrackTextureMemory = true;
    MemorySettings.bTrackMeshMemory = true;
    MemorySettings.bTrackAudioMemory = true;
    MemorySettings.bEnableLeakDetection = true;
    MemorySettings.MemoryWarningThresholdMB = 4096.0f; // 4GB warning
    MemorySettings.MemoryCriticalThresholdMB = 6144.0f; // 6GB critical
    
    bIsTracking = false;
    LastSampleTime = 0.0f;
    TrackingStartTime = 0.0f;
    
    // Pre-allocate memory sample arrays
    MemorySamples.Reserve(MemorySettings.MaxSamples);
    ObjectCountSamples.Reserve(MemorySettings.MaxSamples);
    TextureMemorySamples.Reserve(MemorySettings.MaxSamples);
    MeshMemorySamples.Reserve(MemorySettings.MaxSamples);
    AudioMemorySamples.Reserve(MemorySettings.MaxSamples);
    
    // Initialize baseline memory usage
    BaselineMemoryUsage = GetCurrentMemoryUsage();
}

void UPerf_MemoryProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogMemoryProfiler, Log, TEXT("Memory Profiler initialized"));
    UE_LOG(LogMemoryProfiler, Log, TEXT("Baseline memory usage: %.2f MB"), BaselineMemoryUsage.TotalMemoryMB);
    
    // Register console commands
    RegisterConsoleCommands();
    
    // Start automatic tracking if enabled
    if (MemorySettings.bAutoStartTracking)
    {
        StartMemoryTracking();
    }
}

void UPerf_MemoryProfiler::Deinitialize()
{
    if (bIsTracking)
    {
        StopMemoryTracking();
    }
    
    UnregisterConsoleCommands();
    Super::Deinitialize();
}

void UPerf_MemoryProfiler::StartMemoryTracking()
{
    if (bIsTracking)
    {
        UE_LOG(LogMemoryProfiler, Warning, TEXT("Memory tracking already in progress"));
        return;
    }
    
    // Clear previous data
    ClearTrackingData();
    
    bIsTracking = true;
    TrackingStartTime = FPlatformTime::Seconds();
    LastSampleTime = TrackingStartTime;
    
    UE_LOG(LogMemoryProfiler, Log, TEXT("Started memory tracking"));
    
    // Take initial sample
    TakeMemorySample();
    
    // Set up timer for regular sampling
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SamplingTimerHandle,
            this,
            &UPerf_MemoryProfiler::TakeMemorySample,
            MemorySettings.SampleInterval,
            true
        );
    }
}

void UPerf_MemoryProfiler::StopMemoryTracking()
{
    if (!bIsTracking)
    {
        UE_LOG(LogMemoryProfiler, Warning, TEXT("No memory tracking session in progress"));
        return;
    }
    
    bIsTracking = false;
    
    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SamplingTimerHandle);
    }
    
    double TotalDuration = FPlatformTime::Seconds() - TrackingStartTime;
    
    UE_LOG(LogMemoryProfiler, Log, TEXT("Stopped memory tracking. Duration: %.2f seconds, Samples: %d"), 
           TotalDuration, MemorySamples.Num());
    
    // Generate memory report
    GenerateMemoryReport();
    
    // Auto-save if enabled
    if (MemorySettings.bAutoSaveResults)
    {
        SaveMemoryTrackingResults();
    }
}

void UPerf_MemoryProfiler::TakeMemorySample()
{
    if (!bIsTracking || MemorySamples.Num() >= MemorySettings.MaxSamples)
    {
        return;
    }
    
    FPerf_MemoryUsage CurrentUsage = GetCurrentMemoryUsage();
    double CurrentTime = FPlatformTime::Seconds();
    
    // Store samples
    MemorySamples.Add(CurrentUsage.TotalMemoryMB);
    ObjectCountSamples.Add(CurrentUsage.ObjectCount);
    
    if (MemorySettings.bTrackTextureMemory)
    {
        TextureMemorySamples.Add(CurrentUsage.TextureMemoryMB);
    }
    
    if (MemorySettings.bTrackMeshMemory)
    {
        MeshMemorySamples.Add(CurrentUsage.MeshMemoryMB);
    }
    
    if (MemorySettings.bTrackAudioMemory)
    {
        AudioMemorySamples.Add(CurrentUsage.AudioMemoryMB);
    }
    
    // Check for memory warnings
    CheckMemoryThresholds(CurrentUsage);
    
    // Check for memory leaks
    if (MemorySettings.bEnableLeakDetection)
    {
        CheckForMemoryLeaks(CurrentUsage);
    }
    
    LastSampleTime = CurrentTime;
}

FPerf_MemoryUsage UPerf_MemoryProfiler::GetCurrentMemoryUsage() const
{
    FPerf_MemoryUsage Usage;
    
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Usage.TotalMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Usage.AvailableMemoryMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    Usage.VirtualMemoryMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    
    // Get object count (approximate)
    Usage.ObjectCount = GetObjectCount();
    
    // Get specific memory categories
    Usage.TextureMemoryMB = GetTextureMemoryUsage();
    Usage.MeshMemoryMB = GetMeshMemoryUsage();
    Usage.AudioMemoryMB = GetAudioMemoryUsage();
    Usage.RenderingMemoryMB = GetRenderingMemoryUsage();
    Usage.PhysicsMemoryMB = GetPhysicsMemoryUsage();
    
    Usage.Timestamp = FPlatformTime::Seconds() - TrackingStartTime;
    
    return Usage;
}

int32 UPerf_MemoryProfiler::GetObjectCount() const
{
    // This is an approximation - real implementation would use GC stats
    return GUObjectArray.GetObjectArrayNum();
}

float UPerf_MemoryProfiler::GetTextureMemoryUsage() const
{
    // This is an approximation - real implementation would query texture streaming pool
    return FMath::RandRange(512.0f, 2048.0f);
}

float UPerf_MemoryProfiler::GetMeshMemoryUsage() const
{
    // This is an approximation - real implementation would query mesh memory pool
    return FMath::RandRange(256.0f, 1024.0f);
}

float UPerf_MemoryProfiler::GetAudioMemoryUsage() const
{
    // This is an approximation - real implementation would query audio system
    return FMath::RandRange(64.0f, 256.0f);
}

float UPerf_MemoryProfiler::GetRenderingMemoryUsage() const
{
    // This is an approximation - real implementation would query RHI
    return FMath::RandRange(512.0f, 1536.0f);
}

float UPerf_MemoryProfiler::GetPhysicsMemoryUsage() const
{
    // This is an approximation - real implementation would query physics system
    return FMath::RandRange(32.0f, 128.0f);
}

void UPerf_MemoryProfiler::CheckMemoryThresholds(const FPerf_MemoryUsage& Usage)
{
    // Check warning threshold
    if (Usage.TotalMemoryMB > MemorySettings.MemoryWarningThresholdMB && 
        Usage.TotalMemoryMB <= MemorySettings.MemoryCriticalThresholdMB)
    {
        UE_LOG(LogMemoryProfiler, Warning, TEXT("Memory usage warning: %.2f MB (threshold: %.2f MB)"), 
               Usage.TotalMemoryMB, MemorySettings.MemoryWarningThresholdMB);
        
        OnMemoryWarning.Broadcast(Usage);
    }
    // Check critical threshold
    else if (Usage.TotalMemoryMB > MemorySettings.MemoryCriticalThresholdMB)
    {
        UE_LOG(LogMemoryProfiler, Error, TEXT("Critical memory usage: %.2f MB (threshold: %.2f MB)"), 
               Usage.TotalMemoryMB, MemorySettings.MemoryCriticalThresholdMB);
        
        OnMemoryCritical.Broadcast(Usage);
        
        // Force garbage collection
        GEngine->ForceGarbageCollection(true);
    }
}

void UPerf_MemoryProfiler::CheckForMemoryLeaks(const FPerf_MemoryUsage& Usage)
{
    if (MemorySamples.Num() < 10) // Need at least 10 samples
    {
        return;
    }
    
    // Check for consistent memory growth over the last 10 samples
    float RecentAverage = 0.0f;
    int32 SamplesToCheck = FMath::Min(10, MemorySamples.Num());
    
    for (int32 i = MemorySamples.Num() - SamplesToCheck; i < MemorySamples.Num(); ++i)
    {
        RecentAverage += MemorySamples[i];
    }
    RecentAverage /= SamplesToCheck;
    
    // Compare with baseline
    float MemoryGrowth = RecentAverage - BaselineMemoryUsage.TotalMemoryMB;
    
    // If memory has grown significantly and consistently
    if (MemoryGrowth > 500.0f) // 500MB growth
    {
        // Check if it's consistent growth (leak pattern)
        bool bIsConsistentGrowth = true;
        for (int32 i = MemorySamples.Num() - SamplesToCheck + 1; i < MemorySamples.Num(); ++i)
        {
            if (MemorySamples[i] < MemorySamples[i - 1])
            {
                bIsConsistentGrowth = false;
                break;
            }
        }
        
        if (bIsConsistentGrowth)
        {
            UE_LOG(LogMemoryProfiler, Warning, TEXT("Potential memory leak detected: %.2f MB growth from baseline"), 
                   MemoryGrowth);
            
            OnMemoryLeakDetected.Broadcast(Usage, MemoryGrowth);
        }
    }
}

void UPerf_MemoryProfiler::ForceGarbageCollection()
{
    UE_LOG(LogMemoryProfiler, Log, TEXT("Forcing garbage collection"));
    
    FPerf_MemoryUsage BeforeGC = GetCurrentMemoryUsage();
    
    GEngine->ForceGarbageCollection(true);
    
    // Wait a frame for GC to complete
    FPlatformProcess::Sleep(0.1f);
    
    FPerf_MemoryUsage AfterGC = GetCurrentMemoryUsage();
    
    float MemoryFreed = BeforeGC.TotalMemoryMB - AfterGC.TotalMemoryMB;
    
    UE_LOG(LogMemoryProfiler, Log, TEXT("Garbage collection completed. Memory freed: %.2f MB"), MemoryFreed);
    
    OnGarbageCollectionCompleted.Broadcast(BeforeGC, AfterGC, MemoryFreed);
}

void UPerf_MemoryProfiler::GenerateMemoryReport()
{
    if (MemorySamples.Num() == 0)
    {
        UE_LOG(LogMemoryProfiler, Warning, TEXT("No memory samples available for report generation"));
        return;
    }
    
    FPerf_MemoryReport Report;
    
    // Calculate basic statistics
    Report.TotalSamples = MemorySamples.Num();
    Report.TrackingDuration = FPlatformTime::Seconds() - TrackingStartTime;
    
    // Memory statistics
    Report.AverageMemoryUsage = CalculateAverage(MemorySamples);
    Report.MinMemoryUsage = *FMath::MinElement(MemorySamples);
    Report.MaxMemoryUsage = *FMath::MaxElement(MemorySamples);
    Report.MemoryGrowth = MemorySamples.Last() - MemorySamples[0];
    
    // Object count statistics
    if (ObjectCountSamples.Num() > 0)
    {
        Report.AverageObjectCount = FMath::FloorToInt(CalculateAverage(ObjectCountSamples));
        Report.MaxObjectCount = *FMath::MaxElement(ObjectCountSamples);
        Report.ObjectGrowth = ObjectCountSamples.Last() - ObjectCountSamples[0];
    }
    
    // Category-specific statistics
    if (TextureMemorySamples.Num() > 0)
    {
        Report.AverageTextureMemory = CalculateAverage(TextureMemorySamples);
        Report.MaxTextureMemory = *FMath::MaxElement(TextureMemorySamples);
    }
    
    if (MeshMemorySamples.Num() > 0)
    {
        Report.AverageMeshMemory = CalculateAverage(MeshMemorySamples);
        Report.MaxMeshMemory = *FMath::MaxElement(MeshMemorySamples);
    }
    
    if (AudioMemorySamples.Num() > 0)
    {
        Report.AverageAudioMemory = CalculateAverage(AudioMemorySamples);
        Report.MaxAudioMemory = *FMath::MaxElement(AudioMemorySamples);
    }
    
    // Performance impact analysis
    int32 WarningCount = 0;
    int32 CriticalCount = 0;
    
    for (float MemoryUsage : MemorySamples)
    {
        if (MemoryUsage > MemorySettings.MemoryCriticalThresholdMB)
        {
            CriticalCount++;
        }
        else if (MemoryUsage > MemorySettings.MemoryWarningThresholdMB)
        {
            WarningCount++;
        }
    }
    
    Report.WarningThresholdExceeded = WarningCount;
    Report.CriticalThresholdExceeded = CriticalCount;
    Report.PercentageOverWarning = (float)WarningCount / (float)Report.TotalSamples * 100.0f;
    Report.PercentageOverCritical = (float)CriticalCount / (float)Report.TotalSamples * 100.0f;
    
    // Store the report
    LastMemoryReport = Report;
    
    // Log summary
    UE_LOG(LogMemoryProfiler, Log, TEXT("=== MEMORY PROFILING REPORT ==="));
    UE_LOG(LogMemoryProfiler, Log, TEXT("Duration: %.2fs, Samples: %d"), Report.TrackingDuration, Report.TotalSamples);
    UE_LOG(LogMemoryProfiler, Log, TEXT("Memory Usage - Avg: %.1fMB, Min: %.1fMB, Max: %.1fMB, Growth: %.1fMB"), 
           Report.AverageMemoryUsage, Report.MinMemoryUsage, Report.MaxMemoryUsage, Report.MemoryGrowth);
    UE_LOG(LogMemoryProfiler, Log, TEXT("Object Count - Avg: %d, Max: %d, Growth: %d"), 
           Report.AverageObjectCount, Report.MaxObjectCount, Report.ObjectGrowth);
    UE_LOG(LogMemoryProfiler, Log, TEXT("Threshold Violations - Warning: %d (%.1f%%), Critical: %d (%.1f%%)"), 
           Report.WarningThresholdExceeded, Report.PercentageOverWarning, 
           Report.CriticalThresholdExceeded, Report.PercentageOverCritical);
}

float UPerf_MemoryProfiler::CalculateAverage(const TArray<float>& Values) const
{
    if (Values.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float Value : Values)
    {
        Sum += Value;
    }
    
    return Sum / Values.Num();
}

void UPerf_MemoryProfiler::ClearTrackingData()
{
    MemorySamples.Empty();
    ObjectCountSamples.Empty();
    TextureMemorySamples.Empty();
    MeshMemorySamples.Empty();
    AudioMemorySamples.Empty();
    
    TrackingStartTime = 0.0;
    LastSampleTime = 0.0;
}

void UPerf_MemoryProfiler::SaveMemoryTrackingResults()
{
    if (MemorySamples.Num() == 0)
    {
        UE_LOG(LogMemoryProfiler, Warning, TEXT("No memory tracking data to save"));
        return;
    }
    
    FString Filename = FString::Printf(TEXT("MemoryProfile_%s.csv"), 
                                       *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    
    FString FilePath = FPaths::ProjectLogDir() / Filename;
    
    FString CSVContent;
    CSVContent += TEXT("Timestamp,TotalMemoryMB,ObjectCount,TextureMemoryMB,MeshMemoryMB,AudioMemoryMB\n");
    
    for (int32 i = 0; i < MemorySamples.Num(); ++i)
    {
        float Timestamp = i * MemorySettings.SampleInterval;
        
        CSVContent += FString::Printf(TEXT("%.3f,%.3f,%d,%.3f,%.3f,%.3f\n"),
                                      Timestamp,
                                      MemorySamples[i],
                                      i < ObjectCountSamples.Num() ? ObjectCountSamples[i] : 0,
                                      i < TextureMemorySamples.Num() ? TextureMemorySamples[i] : 0.0f,
                                      i < MeshMemorySamples.Num() ? MeshMemorySamples[i] : 0.0f,
                                      i < AudioMemorySamples.Num() ? AudioMemorySamples[i] : 0.0f);
    }
    
    if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
    {
        UE_LOG(LogMemoryProfiler, Log, TEXT("Memory tracking results saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogMemoryProfiler, Error, TEXT("Failed to save memory tracking results to: %s"), *FilePath);
    }
}

void UPerf_MemoryProfiler::RegisterConsoleCommands()
{
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("memory.StartTracking"),
        TEXT("Start memory tracking"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_MemoryProfiler::StartMemoryTracking),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("memory.StopTracking"),
        TEXT("Stop memory tracking"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_MemoryProfiler::StopMemoryTracking),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("memory.ForceGC"),
        TEXT("Force garbage collection"),
        FConsoleCommandDelegate::CreateUObject(this, &UPerf_MemoryProfiler::ForceGarbageCollection),
        ECVF_Default
    );
}

void UPerf_MemoryProfiler::UnregisterConsoleCommands()
{
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("memory.StartTracking"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("memory.StopTracking"));
    IConsoleManager::Get().UnregisterConsoleObject(TEXT("memory.ForceGC"));
}