#include "Crowd_PerformanceManager.h"
#include "Crowd_MassEntitySubsystem.h"
#include "Crowd_LODSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UCrowd_PerformanceManager::UCrowd_PerformanceManager()
{
    // Initialize frame time history
    FrameTimeHistory.SetNum(FrameTimeHistorySize);
    for (int32 i = 0; i < FrameTimeHistorySize; i++)
    {
        FrameTimeHistory[i] = 16.67f; // Default to 60 FPS
    }

    // Set default adaptive settings
    AdaptiveSettings.TargetFrameTime = 16.67f;
    AdaptiveSettings.MaxFrameTime = 33.33f;
    AdaptiveSettings.MaxActiveEntities = 5000;
    AdaptiveSettings.MinActiveEntities = 500;
    AdaptiveSettings.LODDistanceMultiplier = 1.0f;
    AdaptiveSettings.UpdateFrequencyMultiplier = 1.0f;

    CurrentPerformanceLevel = ECrowd_PerformanceLevel::Medium;
    bAdaptivePerformanceEnabled = true;
    OptimizationInterval = 2.0f;
}

void UCrowd_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get references to other subsystems
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UCrowd_MassEntitySubsystem>();
        LODSystem = World->GetSubsystem<UCrowd_LODSystem>();
    }

    UE_LOG(LogTemp, Log, TEXT("Crowd_PerformanceManager initialized"));
}

void UCrowd_PerformanceManager::Deinitialize()
{
    MassEntitySubsystem = nullptr;
    LODSystem = nullptr;

    Super::Deinitialize();
}

void UCrowd_PerformanceManager::UpdatePerformanceMetrics()
{
    // Update frame time history
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    FrameTimeHistory[FrameTimeIndex] = CurrentFrameTime;
    FrameTimeIndex = (FrameTimeIndex + 1) % FrameTimeHistorySize;

    // Calculate average frame time
    CalculateAverageFrameTime();

    // Update entity counts from Mass Entity subsystem
    if (MassEntitySubsystem)
    {
        CurrentMetrics.ActiveEntities = MassEntitySubsystem->GetActiveEntityCount();
        CurrentMetrics.VisibleEntities = MassEntitySubsystem->GetVisibleEntityCount();
    }

    // Update LOD counts from LOD system
    if (LODSystem)
    {
        CurrentMetrics.LODLevel0Count = LODSystem->GetLODLevelCount(0);
        CurrentMetrics.LODLevel1Count = LODSystem->GetLODLevelCount(1);
        CurrentMetrics.LODLevel2Count = LODSystem->GetLODLevelCount(2);
        CurrentMetrics.CulledCount = LODSystem->GetCulledEntityCount();
    }

    // Check if we need to optimize performance
    if (bAdaptivePerformanceEnabled)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastOptimizationTime > OptimizationInterval)
        {
            CheckPerformanceThresholds();
            LastOptimizationTime = CurrentTime;
        }
    }
}

void UCrowd_PerformanceManager::SetAdaptiveSettings(const FCrowd_AdaptiveSettings& NewSettings)
{
    AdaptiveSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Crowd performance adaptive settings updated"));
}

void UCrowd_PerformanceManager::OptimizePerformance()
{
    if (!bAdaptivePerformanceEnabled)
    {
        return;
    }

    CheckPerformanceThresholds();
    ApplyPerformanceOptimizations();
}

void UCrowd_PerformanceManager::SetPerformanceLevel(ECrowd_PerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;

    switch (Level)
    {
    case ECrowd_PerformanceLevel::Low:
        AdaptiveSettings.MaxActiveEntities = 1000;
        AdaptiveSettings.LODDistanceMultiplier = 0.5f;
        AdaptiveSettings.UpdateFrequencyMultiplier = 0.5f;
        break;

    case ECrowd_PerformanceLevel::Medium:
        AdaptiveSettings.MaxActiveEntities = 3000;
        AdaptiveSettings.LODDistanceMultiplier = 1.0f;
        AdaptiveSettings.UpdateFrequencyMultiplier = 1.0f;
        break;

    case ECrowd_PerformanceLevel::High:
        AdaptiveSettings.MaxActiveEntities = 5000;
        AdaptiveSettings.LODDistanceMultiplier = 1.5f;
        AdaptiveSettings.UpdateFrequencyMultiplier = 1.5f;
        break;

    case ECrowd_PerformanceLevel::Ultra:
        AdaptiveSettings.MaxActiveEntities = 10000;
        AdaptiveSettings.LODDistanceMultiplier = 2.0f;
        AdaptiveSettings.UpdateFrequencyMultiplier = 2.0f;
        break;
    }

    ApplyPerformanceOptimizations();
    UE_LOG(LogTemp, Log, TEXT("Crowd performance level set to: %d"), (int32)Level);
}

void UCrowd_PerformanceManager::ReduceCrowdDensity(float Percentage)
{
    if (MassEntitySubsystem)
    {
        int32 CurrentCount = MassEntitySubsystem->GetActiveEntityCount();
        int32 NewCount = FMath::Max(AdaptiveSettings.MinActiveEntities, 
                                   (int32)(CurrentCount * (1.0f - Percentage)));
        
        MassEntitySubsystem->SetMaxEntityCount(NewCount);
        UE_LOG(LogTemp, Log, TEXT("Reduced crowd density by %.1f%% to %d entities"), 
               Percentage * 100.0f, NewCount);
    }
}

void UCrowd_PerformanceManager::IncreaseCrowdDensity(float Percentage)
{
    if (MassEntitySubsystem)
    {
        int32 CurrentCount = MassEntitySubsystem->GetActiveEntityCount();
        int32 NewCount = FMath::Min(AdaptiveSettings.MaxActiveEntities, 
                                   (int32)(CurrentCount * (1.0f + Percentage)));
        
        MassEntitySubsystem->SetMaxEntityCount(NewCount);
        UE_LOG(LogTemp, Log, TEXT("Increased crowd density by %.1f%% to %d entities"), 
               Percentage * 100.0f, NewCount);
    }
}

void UCrowd_PerformanceManager::SetMaxEntityCount(int32 MaxCount)
{
    AdaptiveSettings.MaxActiveEntities = FMath::Max(AdaptiveSettings.MinActiveEntities, MaxCount);
    
    if (MassEntitySubsystem)
    {
        MassEntitySubsystem->SetMaxEntityCount(AdaptiveSettings.MaxActiveEntities);
    }
}

void UCrowd_PerformanceManager::CalculateAverageFrameTime()
{
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    CurrentMetrics.AverageFrameTime = Total / FrameTimeHistorySize;
}

void UCrowd_PerformanceManager::CheckPerformanceThresholds()
{
    float AvgFrameTime = GetAverageFrameTime();

    if (AvgFrameTime > PoorPerformanceThreshold)
    {
        // Performance is poor, reduce quality
        if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::Ultra)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::High);
        }
        else if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::High)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::Medium);
        }
        else if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::Medium)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::Low);
        }
        else
        {
            // Already at lowest level, reduce entity count
            ReduceCrowdDensity(0.1f);
        }
    }
    else if (AvgFrameTime < GoodPerformanceThreshold)
    {
        // Performance is good, can increase quality
        if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::Low)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::Medium);
        }
        else if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::Medium)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::High);
        }
        else if (CurrentPerformanceLevel == ECrowd_PerformanceLevel::High)
        {
            SetPerformanceLevel(ECrowd_PerformanceLevel::Ultra);
        }
        else
        {
            // Already at highest level, can increase entity count slightly
            IncreaseCrowdDensity(0.05f);
        }
    }
}

void UCrowd_PerformanceManager::ApplyPerformanceOptimizations()
{
    UpdateLODSettings();
    UpdateEntityCounts();
}

void UCrowd_PerformanceManager::UpdateLODSettings()
{
    if (LODSystem)
    {
        // Adjust LOD distances based on performance level
        float BaseDistance = 1000.0f;
        float Multiplier = AdaptiveSettings.LODDistanceMultiplier;

        LODSystem->SetLODDistance(0, BaseDistance * 0.3f * Multiplier);
        LODSystem->SetLODDistance(1, BaseDistance * 0.6f * Multiplier);
        LODSystem->SetLODDistance(2, BaseDistance * 1.0f * Multiplier);
    }
}

void UCrowd_PerformanceManager::UpdateEntityCounts()
{
    if (MassEntitySubsystem)
    {
        MassEntitySubsystem->SetMaxEntityCount(AdaptiveSettings.MaxActiveEntities);
        MassEntitySubsystem->SetUpdateFrequencyMultiplier(AdaptiveSettings.UpdateFrequencyMultiplier);
    }
}

float UCrowd_PerformanceManager::GetAverageFrameTime() const
{
    return CurrentMetrics.AverageFrameTime;
}

bool UCrowd_PerformanceManager::IsPerformancePoor() const
{
    return GetAverageFrameTime() > PoorPerformanceThreshold;
}

bool UCrowd_PerformanceManager::IsPerformanceGood() const
{
    return GetAverageFrameTime() < GoodPerformanceThreshold;
}