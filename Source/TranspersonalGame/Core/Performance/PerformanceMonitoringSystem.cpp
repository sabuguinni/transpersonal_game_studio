#include "PerformanceMonitoringSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Components/SceneComponent.h"

APerf_PerformanceMonitoringSystem::APerf_PerformanceMonitoringSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    MaxMemoryUsageMB = 4096.0f;
    MaxDrawCalls = 2000;
    bAutomaticQualityAdjustment = true;
    MetricsUpdateInterval = 1.0f;
    LastMetricsUpdate = 0.0f;

    // Initialize LOD settings
    CurrentLODSettings.LODDistanceScale = 1.0f;
    CurrentLODSettings.CullingDistanceScale = 1.0f;
    CurrentLODSettings.MaxLODLevel = 3;
    CurrentLODSettings.bEnableAutomaticLOD = true;

    // Initialize FPS history
    FPSHistory.Reserve(MaxFPSHistorySize);
}

void APerf_PerformanceMonitoringSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("Performance Monitoring System initialized"));

    // Register with world subsystem
    if (UPerf_PerformanceWorldSubsystem* PerfSubsystem = UPerf_PerformanceWorldSubsystem::Get(this))
    {
        PerfSubsystem->RegisterPerformanceMonitor(this);
    }

    // Apply initial performance settings
    ApplyPerformanceLevelSettings(EPerf_PerformanceLevel::High);
}

void APerf_PerformanceMonitoringSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    LastMetricsUpdate += DeltaTime;

    // Update metrics at specified interval
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bAutomaticQualityAdjustment)
        {
            AdjustQualitySettings();
        }

        LastMetricsUpdate = 0.0f;
    }
}

void APerf_PerformanceMonitoringSystem::UpdatePerformanceMetrics()
{
    // Get current FPS
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentFPS = FMath::Clamp(CurrentFPS, 1.0f, 300.0f); // Reasonable bounds

    // Update FPS history
    UpdateFPSHistory(CurrentFPS);

    // Calculate metrics
    CurrentMetrics.CurrentFPS = CurrentFPS;
    CurrentMetrics.AverageFPS = CalculateAverageFPS();
    CurrentMetrics.FrameTime = (1.0f / CurrentFPS) * 1000.0f; // Convert to milliseconds

    // Get memory usage (approximation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Get visible actors count
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            if (ActorItr->WasRecentlyRendered(0.1f))
            {
                ActorCount++;
            }
        }
        CurrentMetrics.VisibleActors = ActorCount;
    }

    // Determine performance level based on current FPS
    if (CurrentMetrics.AverageFPS >= 55.0f)
    {
        CurrentMetrics.CurrentLevel = EPerf_PerformanceLevel::Ultra;
    }
    else if (CurrentMetrics.AverageFPS >= 45.0f)
    {
        CurrentMetrics.CurrentLevel = EPerf_PerformanceLevel::High;
    }
    else if (CurrentMetrics.AverageFPS >= 35.0f)
    {
        CurrentMetrics.CurrentLevel = EPerf_PerformanceLevel::Medium;
    }
    else if (CurrentMetrics.AverageFPS >= 25.0f)
    {
        CurrentMetrics.CurrentLevel = EPerf_PerformanceLevel::Low;
    }
    else
    {
        CurrentMetrics.CurrentLevel = EPerf_PerformanceLevel::Potato;
    }
}

void APerf_PerformanceMonitoringSystem::AdjustQualitySettings()
{
    static EPerf_PerformanceLevel LastLevel = EPerf_PerformanceLevel::High;
    
    // Only adjust if performance level has changed
    if (CurrentMetrics.CurrentLevel != LastLevel)
    {
        ApplyPerformanceLevelSettings(CurrentMetrics.CurrentLevel);
        LastLevel = CurrentMetrics.CurrentLevel;
        
        UE_LOG(LogTemp, Warning, TEXT("Performance level adjusted to: %d"), (int32)CurrentMetrics.CurrentLevel);
    }
}

void APerf_PerformanceMonitoringSystem::ApplyPerformanceLevelSettings(EPerf_PerformanceLevel Level)
{
    if (!GetWorld())
    {
        return;
    }

    switch (Level)
    {
        case EPerf_PerformanceLevel::Ultra:
            // Ultra quality settings
            GetWorld()->Exec(GetWorld(), TEXT("r.ShadowQuality 4"));
            GetWorld()->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 6"));
            GetWorld()->Exec(GetWorld(), TEXT("r.DetailMode 2"));
            GetWorld()->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
            CurrentLODSettings.LODDistanceScale = 1.0f;
            break;

        case EPerf_PerformanceLevel::High:
            // High quality settings
            GetWorld()->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
            GetWorld()->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 4"));
            GetWorld()->Exec(GetWorld(), TEXT("r.DetailMode 2"));
            GetWorld()->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.9"));
            CurrentLODSettings.LODDistanceScale = 1.1f;
            break;

        case EPerf_PerformanceLevel::Medium:
            // Medium quality settings
            GetWorld()->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
            GetWorld()->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 2"));
            GetWorld()->Exec(GetWorld(), TEXT("r.DetailMode 1"));
            GetWorld()->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
            CurrentLODSettings.LODDistanceScale = 1.3f;
            break;

        case EPerf_PerformanceLevel::Low:
            // Low quality settings
            GetWorld()->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
            GetWorld()->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.DetailMode 0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.6"));
            CurrentLODSettings.LODDistanceScale = 1.5f;
            break;

        case EPerf_PerformanceLevel::Potato:
            // Potato quality settings
            GetWorld()->Exec(GetWorld(), TEXT("r.ShadowQuality 0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.PostProcessAAQuality 0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.DetailMode 0"));
            GetWorld()->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.4"));
            CurrentLODSettings.LODDistanceScale = 2.0f;
            break;
    }

    // Apply LOD settings
    ApplyLODSettings(CurrentLODSettings);
}

FPerf_PerformanceMetrics APerf_PerformanceMonitoringSystem::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_PerformanceMonitoringSystem::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    ApplyPerformanceLevelSettings(NewLevel);
    CurrentMetrics.CurrentLevel = NewLevel;
}

void APerf_PerformanceMonitoringSystem::EnableAutomaticQualityAdjustment(bool bEnable)
{
    bAutomaticQualityAdjustment = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Automatic quality adjustment: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void APerf_PerformanceMonitoringSystem::ApplyLODSettings(const FPerf_LODSettings& Settings)
{
    CurrentLODSettings = Settings;

    if (UWorld* World = GetWorld())
    {
        // Apply LOD distance scale
        FString Command = FString::Printf(TEXT("r.StaticMeshLODDistanceScale %f"), Settings.LODDistanceScale);
        World->Exec(World, *Command);

        // Apply culling distance scale
        Command = FString::Printf(TEXT("r.CullDistanceScale %f"), Settings.CullingDistanceScale);
        World->Exec(World, *Command);

        // Force LOD level if not automatic
        if (!Settings.bEnableAutomaticLOD)
        {
            Command = FString::Printf(TEXT("r.ForceLOD %d"), Settings.MaxLODLevel);
            World->Exec(World, *Command);
        }
        else
        {
            World->Exec(World, TEXT("r.ForceLOD -1")); // Enable automatic LOD
        }
    }
}

void APerf_PerformanceMonitoringSystem::ForceGarbageCollection()
{
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Log, TEXT("Forced garbage collection"));
    }
}

float APerf_PerformanceMonitoringSystem::CalculateAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 60.0f; // Default
    }

    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }

    return Sum / FPSHistory.Num();
}

void APerf_PerformanceMonitoringSystem::UpdateFPSHistory(float CurrentFPS)
{
    FPSHistory.Add(CurrentFPS);

    // Keep history size within bounds
    if (FPSHistory.Num() > MaxFPSHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
}

// World Subsystem Implementation
void UPerf_PerformanceWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    PerformanceMonitor = nullptr;
}

void UPerf_PerformanceWorldSubsystem::Deinitialize()
{
    PerformanceMonitor = nullptr;
    Super::Deinitialize();
}

UPerf_PerformanceWorldSubsystem* UPerf_PerformanceWorldSubsystem::Get(const UObject* WorldContext)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
    {
        return World->GetSubsystem<UPerf_PerformanceWorldSubsystem>();
    }
    return nullptr;
}

APerf_PerformanceMonitoringSystem* UPerf_PerformanceWorldSubsystem::GetPerformanceMonitor() const
{
    return PerformanceMonitor;
}

void UPerf_PerformanceWorldSubsystem::RegisterPerformanceMonitor(APerf_PerformanceMonitoringSystem* Monitor)
{
    PerformanceMonitor = Monitor;
    UE_LOG(LogTemp, Log, TEXT("Performance monitor registered with world subsystem"));
}