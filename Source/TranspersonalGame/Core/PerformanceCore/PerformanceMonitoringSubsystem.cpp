#include "PerformanceMonitoringSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

UPerformanceMonitoringSubsystem::UPerformanceMonitoringSubsystem()
    : CurrentFPS(60.0f)
    , AverageFPS(60.0f)
    , TargetFPS(60.0f)
    , CurrentMemoryUsageMB(0.0f)
    , CurrentGPUTime(0.0f)
    , CurrentPerformanceLevel(EPerf_PerformanceLevel::High)
    , bAutoQualityAdjustment(true)
    , bIsMonitoring(false)
    , MonitoringInterval(0.5f)
    , FrameRateAccumulator(0.0f)
    , FrameRateSamples(0)
    , LastUpdateTime(0.0)
{
    FrameRateHistory.Reserve(MaxFrameRateHistory);
}

void UPerformanceMonitoringSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Initializing"));
    
    // Set default target frame rate based on platform
    #if PLATFORM_DESKTOP
        TargetFPS = 60.0f;
        CurrentPerformanceLevel = EPerf_PerformanceLevel::High;
    #else
        TargetFPS = 30.0f;
        CurrentPerformanceLevel = EPerf_PerformanceLevel::Medium;
    #endif
    
    // Start monitoring automatically
    StartPerformanceMonitoring();
}

void UPerformanceMonitoringSubsystem::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UPerformanceMonitoringSubsystem::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Starting performance monitoring"));
    
    bIsMonitoring = true;
    LastUpdateTime = FPlatformTime::Seconds();
    
    // Set up timer for regular monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerformanceMonitoringSubsystem::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
    }
}

void UPerformanceMonitoringSubsystem::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Stopping performance monitoring"));
    
    bIsMonitoring = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
    }
}

void UPerformanceMonitoringSubsystem::SetTargetFrameRate(float InTargetFPS)
{
    TargetFPS = FMath::Clamp(InTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Target FPS set to %.1f"), TargetFPS);
}

void UPerformanceMonitoringSubsystem::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    if (CurrentPerformanceLevel != NewLevel)
    {
        EPerf_PerformanceLevel OldLevel = CurrentPerformanceLevel;
        CurrentPerformanceLevel = NewLevel;
        
        ApplyPerformanceLevel(NewLevel);
        
        UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Performance level changed from %d to %d"), 
               (int32)OldLevel, (int32)NewLevel);
        
        OnPerformanceThresholdExceeded.Broadcast(NewLevel);
    }
}

void UPerformanceMonitoringSubsystem::EnableAutoQualityAdjustment(bool bEnable)
{
    bAutoQualityAdjustment = bEnable;
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Auto quality adjustment %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceMonitoringSubsystem::RegisterPerformanceZone(const FString& ZoneName, const FVector& Location, float Radius, EPerf_PerformanceLevel RequiredLevel)
{
    FPerf_PerformanceZone Zone;
    Zone.ZoneName = ZoneName;
    Zone.Location = Location;
    Zone.Radius = Radius;
    Zone.RequiredPerformanceLevel = RequiredLevel;
    Zone.bIsActive = true;
    
    PerformanceZones.Add(ZoneName, Zone);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Registered performance zone '%s' at %s"), 
           *ZoneName, *Location.ToString());
}

void UPerformanceMonitoringSubsystem::UnregisterPerformanceZone(const FString& ZoneName)
{
    if (PerformanceZones.Remove(ZoneName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Unregistered performance zone '%s'"), *ZoneName);
    }
}

void UPerformanceMonitoringSubsystem::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    double CurrentTime = FPlatformTime::Seconds();
    double DeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;
    
    // Update FPS
    if (DeltaTime > 0.0)
    {
        CurrentFPS = 1.0f / DeltaTime;
        
        // Add to history
        FrameRateHistory.Add(CurrentFPS);
        if (FrameRateHistory.Num() > MaxFrameRateHistory)
        {
            FrameRateHistory.RemoveAt(0);
        }
        
        // Calculate average
        FrameRateAccumulator += CurrentFPS;
        FrameRateSamples++;
        
        if (FrameRateSamples >= 10)
        {
            AverageFPS = FrameRateAccumulator / FrameRateSamples;
            FrameRateAccumulator = 0.0f;
            FrameRateSamples = 0;
        }
    }
    
    // Update memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update GPU time (simplified)
    CurrentGPUTime = FMath::Max(0.0f, (1000.0f / CurrentFPS) - 16.67f); // Rough GPU time estimation
    
    // Check thresholds and adjust quality if needed
    CheckPerformanceThresholds();
    UpdatePerformanceZones();
    
    // Broadcast frame rate changes
    OnFrameRateChanged.Broadcast(CurrentFPS, TargetFPS);
}

void UPerformanceMonitoringSubsystem::CheckPerformanceThresholds()
{
    if (!bAutoQualityAdjustment)
    {
        return;
    }
    
    EPerf_PerformanceLevel NewLevel = CurrentPerformanceLevel;
    
    // Determine performance level based on current FPS
    if (AverageFPS >= HighPerformanceThreshold)
    {
        NewLevel = EPerf_PerformanceLevel::High;
    }
    else if (AverageFPS >= MediumPerformanceThreshold)
    {
        NewLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (AverageFPS >= LowPerformanceThreshold)
    {
        NewLevel = EPerf_PerformanceLevel::Low;
    }
    else
    {
        NewLevel = EPerf_PerformanceLevel::VeryLow;
    }
    
    // Only change if we've been at the current level for a while
    static int32 LevelChangeCounter = 0;
    if (NewLevel != CurrentPerformanceLevel)
    {
        LevelChangeCounter++;
        if (LevelChangeCounter >= 5) // Require 5 consistent readings
        {
            SetPerformanceLevel(NewLevel);
            LevelChangeCounter = 0;
        }
    }
    else
    {
        LevelChangeCounter = 0;
    }
}

void UPerformanceMonitoringSubsystem::AdjustQualitySettings()
{
    ApplyPerformanceLevel(CurrentPerformanceLevel);
}

void UPerformanceMonitoringSubsystem::UpdatePerformanceZones()
{
    // Check if player is in any performance zones
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            for (auto& ZonePair : PerformanceZones)
            {
                FPerf_PerformanceZone& Zone = ZonePair.Value;
                float Distance = FVector::Dist(PlayerLocation, Zone.Location);
                
                if (Distance <= Zone.Radius)
                {
                    // Player is in this zone - ensure performance level meets requirements
                    if (CurrentPerformanceLevel < Zone.RequiredPerformanceLevel)
                    {
                        SetPerformanceLevel(Zone.RequiredPerformanceLevel);
                        UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Entered zone '%s', adjusted to performance level %d"), 
                               *Zone.ZoneName, (int32)Zone.RequiredPerformanceLevel);
                    }
                }
            }
        }
    }
}

void UPerformanceMonitoringSubsystem::ApplyPerformanceLevel(EPerf_PerformanceLevel Level)
{
    switch (Level)
    {
        case EPerf_PerformanceLevel::VeryLow:
            SetLODBias(2.0f);
            SetShadowQuality(0);
            SetTextureQuality(0);
            SetPostProcessQuality(0);
            break;
            
        case EPerf_PerformanceLevel::Low:
            SetLODBias(1.0f);
            SetShadowQuality(1);
            SetTextureQuality(1);
            SetPostProcessQuality(1);
            break;
            
        case EPerf_PerformanceLevel::Medium:
            SetLODBias(0.5f);
            SetShadowQuality(2);
            SetTextureQuality(2);
            SetPostProcessQuality(2);
            break;
            
        case EPerf_PerformanceLevel::High:
            SetLODBias(0.0f);
            SetShadowQuality(3);
            SetTextureQuality(3);
            SetPostProcessQuality(3);
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            SetLODBias(-0.5f);
            SetShadowQuality(4);
            SetTextureQuality(4);
            SetPostProcessQuality(4);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceMonitoringSubsystem: Applied performance level %d"), (int32)Level);
}

void UPerformanceMonitoringSubsystem::SetLODBias(float Bias)
{
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.LODBias %.2f"), Bias);
        GEngine->Exec(World, *Command);
    }
}

void UPerformanceMonitoringSubsystem::SetShadowQuality(int32 Quality)
{
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.ShadowQuality %d"), Quality);
        GEngine->Exec(World, *Command);
    }
}

void UPerformanceMonitoringSubsystem::SetTextureQuality(int32 Quality)
{
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.TextureQuality %d"), Quality);
        GEngine->Exec(World, *Command);
    }
}

void UPerformanceMonitoringSubsystem::SetPostProcessQuality(int32 Quality)
{
    if (UWorld* World = GetWorld())
    {
        FString Command = FString::Printf(TEXT("r.PostProcessAAQuality %d"), Quality);
        GEngine->Exec(World, *Command);
    }
}