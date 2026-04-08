// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "RHI.h"
#include "RenderCore.h"
#include "Stats/StatsData.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance targets
    TargetFPS_PC = 60.0f;
    TargetFPS_Console = 30.0f;
    TargetFPS_Mobile = 30.0f;
    
    FrameTimeBudget_PC = 16.67f;
    FrameTimeBudget_Console = 33.33f;
    
    MaxMemoryUsage_PC = 12000.0f;
    MaxMemoryUsage_Console = 8000.0f;
    
    // Dynamic quality settings
    bDynamicQualityEnabled = true;
    QualityAdjustmentThreshold = 5.0f;
    QualityCheckInterval = 2.0f;
    
    // Jurassic-specific settings
    MaxVisibleDinosaurs = 50;
    DinosaurCullDistance = 2000.0f;
    MaxPhysicsSubsteps = 4;
    VegetationDensityScale = 1.0f;
    
    // Initialize tracking variables
    CurrentFPS = 0.0f;
    CurrentFrameTime = 0.0f;
    CurrentMemoryUsage = 0.0f;
    CurrentQualityLevel = 2; // Start at High
    LastQualityCheckTime = 0.0f;
    
    bIsHighEndPC = false;
    bIsConsole = false;
    bSupportsDLSS = false;
    bSupportsRayTracing = false;
    
    RecentFrameTimes.Reserve(60); // Store last 60 frame times for averaging
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Initializing performance system for Jurassic survival game"));
    
    DetectPlatformCapabilities();
    InitializePerformanceSystem();
    ApplyJurassicOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Performance system initialized successfully"));
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics();
    CheckFrameTimeTarget();
    CheckMemoryUsage();
    
    // Check if we need to adjust quality
    if (bDynamicQualityEnabled)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastQualityCheckTime >= QualityCheckInterval)
        {
            AdjustQualityIfNeeded();
            LastQualityCheckTime = CurrentTime;
        }
    }
}

void UPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Shutting down performance monitoring"));
    Super::EndPlay(EndPlayReason);
}

void UPerformanceManager::InitializePerformanceSystem()
{
    // Set initial quality level based on platform
    if (bIsHighEndPC)
    {
        CurrentQualityLevel = 3; // Epic
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: High-end PC detected, setting Epic quality"));
    }
    else if (bIsConsole)
    {
        CurrentQualityLevel = 2; // High
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Console detected, setting High quality"));
    }
    else
    {
        CurrentQualityLevel = 1; // Medium
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Standard hardware detected, setting Medium quality"));
    }
    
    ApplyQualityLevel(CurrentQualityLevel);
    
    // Initialize console variables for runtime adjustment
    IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("jurassic.QualityLevel"),
        CurrentQualityLevel,
        TEXT("Current quality level (0=Low, 1=Medium, 2=High, 3=Epic)"),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("jurassic.DynamicQuality"),
        bDynamicQualityEnabled ? 1 : 0,
        TEXT("Enable dynamic quality scaling based on performance"),
        ECVF_Default
    );
    
    IConsoleManager::Get().RegisterConsoleVariable(
        TEXT("jurassic.TargetFPS"),
        bIsConsole ? TargetFPS_Console : TargetFPS_PC,
        TEXT("Target frame rate for performance monitoring"),
        ECVF_Default
    );
}

FPerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    FPerformanceMetrics Metrics;
    Metrics.FPS = CurrentFPS;
    Metrics.FrameTime = CurrentFrameTime;
    Metrics.MemoryUsage = CurrentMemoryUsage;
    Metrics.QualityLevel = CurrentQualityLevel;
    
    float TargetFPS = bIsConsole ? TargetFPS_Console : TargetFPS_PC;
    Metrics.bMeetingTargetFPS = CurrentFPS >= (TargetFPS * 0.95f); // 5% tolerance
    
    // Get dinosaur count from game state (placeholder)
    Metrics.VisibleDinosaurs = 0; // Would be populated from actual game state
    
    // Calculate physics load as percentage of frame time
    Metrics.PhysicsLoad = (CurrentFrameTime > 0.0f) ? FMath::Clamp((CurrentFrameTime * 0.3f) / CurrentFrameTime, 0.0f, 1.0f) : 0.0f;
    
    // GPU utilization (placeholder - would use actual GPU stats)
    Metrics.GPUUtilization = FMath::Clamp(CurrentFrameTime / (bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC), 0.0f, 1.0f);
    
    return Metrics;
}

void UPerformanceManager::SetQualityLevel(int32 NewQualityLevel)
{
    NewQualityLevel = FMath::Clamp(NewQualityLevel, 0, 3);
    
    if (NewQualityLevel != CurrentQualityLevel)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Changing quality level from %d to %d"), CurrentQualityLevel, NewQualityLevel);
        CurrentQualityLevel = NewQualityLevel;
        ApplyQualityLevel(CurrentQualityLevel);
    }
}

void UPerformanceManager::SetDynamicQualityEnabled(bool bEnable)
{
    bDynamicQualityEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Dynamic quality scaling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

int32 UPerformanceManager::GetRecommendedQualityLevel() const
{
    if (bIsHighEndPC && bSupportsRayTracing)
    {
        return 3; // Epic
    }
    else if (bIsHighEndPC || bIsConsole)
    {
        return 2; // High
    }
    else
    {
        return 1; // Medium
    }
}

void UPerformanceManager::ApplyJurassicOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applying Jurassic-specific optimizations"));
    
    OptimizeDinosaurRendering();
    OptimizeVegetationSystem();
    OptimizePhysicsSimulation();
    OptimizeMemoryStreaming();
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current FPS and frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
        
        // Store recent frame times for averaging
        RecentFrameTimes.Add(CurrentFrameTime);
        if (RecentFrameTimes.Num() > 60)
        {
            RecentFrameTimes.RemoveAt(0);
        }
    }
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void UPerformanceManager::CheckFrameTimeTarget()
{
    float TargetFrameTime = bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC;
    
    if (CurrentFrameTime > TargetFrameTime + QualityAdjustmentThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Frame time %f ms exceeds target %f ms"), CurrentFrameTime, TargetFrameTime);
    }
}

void UPerformanceManager::CheckMemoryUsage()
{
    float MaxMemory = bIsConsole ? MaxMemoryUsage_Console : MaxMemoryUsage_PC;
    
    if (CurrentMemoryUsage > MaxMemory * 0.9f) // 90% threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Memory usage %f MB approaching limit %f MB"), CurrentMemoryUsage, MaxMemory);
    }
}

void UPerformanceManager::AdjustQualityIfNeeded()
{
    if (RecentFrameTimes.Num() < 30) return; // Need enough samples
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : RecentFrameTimes)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= RecentFrameTimes.Num();
    
    float TargetFrameTime = bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC;
    
    // Check if we need to reduce quality
    if (AverageFrameTime > TargetFrameTime + QualityAdjustmentThreshold && CurrentQualityLevel > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Reducing quality level due to poor performance"));
        SetQualityLevel(CurrentQualityLevel - 1);
    }
    // Check if we can increase quality
    else if (AverageFrameTime < TargetFrameTime - QualityAdjustmentThreshold && CurrentQualityLevel < 3)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Increasing quality level due to good performance"));
        SetQualityLevel(CurrentQualityLevel + 1);
    }
}

void UPerformanceManager::DetectPlatformCapabilities()
{
    // Detect platform type
    FString PlatformName = FPlatformProperties::PlatformName();
    
    if (PlatformName == TEXT("Windows") || PlatformName == TEXT("Mac") || PlatformName == TEXT("Linux"))
    {
        // Check if it's high-end PC based on GPU and memory
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        float TotalMemoryGB = MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f);
        
        bIsHighEndPC = TotalMemoryGB >= 16.0f; // 16GB+ RAM indicates high-end
        bIsConsole = false;
        
        // Check for ray tracing support (placeholder)
        bSupportsRayTracing = GRHISupportsRayTracing;
        bSupportsDLSS = false; // Would check for DLSS support
    }
    else
    {
        bIsConsole = true;
        bIsHighEndPC = false;
        bSupportsRayTracing = true; // Consoles support ray tracing
        bSupportsDLSS = false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Platform detected - HighEndPC: %s, Console: %s, RayTracing: %s"), 
           bIsHighEndPC ? TEXT("Yes") : TEXT("No"),
           bIsConsole ? TEXT("Yes") : TEXT("No"),
           bSupportsRayTracing ? TEXT("Yes") : TEXT("No"));
}

bool UPerformanceManager::IsHighEndPC() const
{
    return bIsHighEndPC;
}

bool UPerformanceManager::IsConsole() const
{
    return bIsConsole;
}

void UPerformanceManager::ApplyQualityLevel(int32 QualityLevel)
{
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applying quality level %d"), QualityLevel);
    
    SetRenderingQuality(QualityLevel);
    SetPhysicsQuality(QualityLevel);
    SetAudioQuality(QualityLevel);
}

void UPerformanceManager::SetRenderingQuality(int32 Level)
{
    // Apply scalability settings using console commands
    FString QualityCommands[] = {
        TEXT("sg.ViewDistanceQuality 0"),     // Low
        TEXT("sg.ViewDistanceQuality 1"),     // Medium
        TEXT("sg.ViewDistanceQuality 2"),     // High
        TEXT("sg.ViewDistanceQuality 3")      // Epic
    };
    
    if (Level >= 0 && Level <= 3)
    {
        // Apply view distance quality
        GetWorld()->Exec(GetWorld(), *QualityCommands[Level]);
        
        // Apply other rendering settings
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("sg.AntiAliasingQuality %d"), Level));
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("sg.PostProcessQuality %d"), Level));
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("sg.ShadowQuality %d"), Level));
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("sg.TextureQuality %d"), Level));
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("sg.EffectsQuality %d"), Level));
        
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applied rendering quality level %d"), Level);
    }
}

void UPerformanceManager::SetPhysicsQuality(int32 Level)
{
    // Adjust physics settings based on quality level
    switch (Level)
    {
        case 0: // Low
            MaxPhysicsSubsteps = 2;
            GetWorld()->Exec(GetWorld(), TEXT("p.Chaos.Solver.IterationCount 4"));
            break;
        case 1: // Medium
            MaxPhysicsSubsteps = 3;
            GetWorld()->Exec(GetWorld(), TEXT("p.Chaos.Solver.IterationCount 6"));
            break;
        case 2: // High
            MaxPhysicsSubsteps = 4;
            GetWorld()->Exec(GetWorld(), TEXT("p.Chaos.Solver.IterationCount 8"));
            break;
        case 3: // Epic
            MaxPhysicsSubsteps = 6;
            GetWorld()->Exec(GetWorld(), TEXT("p.Chaos.Solver.IterationCount 10"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applied physics quality level %d (MaxSubsteps: %d)"), Level, MaxPhysicsSubsteps);
}

void UPerformanceManager::SetAudioQuality(int32 Level)
{
    // Adjust audio quality settings
    int32 MaxChannels[] = {32, 64, 96, 128}; // Low to Epic
    
    if (Level >= 0 && Level <= 3)
    {
        GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("au.MaxChannels %d"), MaxChannels[Level]));
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Applied audio quality level %d (%d channels)"), Level, MaxChannels[Level]);
    }
}

void UPerformanceManager::OptimizeDinosaurRendering()
{
    // Set dinosaur-specific rendering optimizations
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.MaxAnisotropy %d"), CurrentQualityLevel + 1));
    
    // Adjust LOD bias for creatures
    float LODBias = (3 - CurrentQualityLevel) * 0.5f; // Higher quality = lower LOD bias
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.SkeletalMeshLODBias %f"), LODBias));
    
    // Set cull distance for dinosaurs
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.ViewDistanceScale %f"), 
                     FMath::Lerp(0.6f, 1.0f, CurrentQualityLevel / 3.0f)));
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Optimized dinosaur rendering (LODBias: %f)"), LODBias);
}

void UPerformanceManager::OptimizeVegetationSystem()
{
    // Adjust vegetation density based on quality level
    VegetationDensityScale = FMath::Lerp(0.3f, 1.0f, CurrentQualityLevel / 3.0f);
    
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("foliage.DensityScale %f"), VegetationDensityScale));
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("grass.DensityMultiplier %f"), VegetationDensityScale));
    
    // Adjust foliage cull distance
    float FoliageCullDistance = FMath::Lerp(1000.0f, 3000.0f, CurrentQualityLevel / 3.0f);
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("foliage.MaxTrianglesToRender %d"), 
                     FMath::RoundToInt(FMath::Lerp(50000.0f, 200000.0f, CurrentQualityLevel / 3.0f))));
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Optimized vegetation system (Density: %f)"), VegetationDensityScale);
}

void UPerformanceManager::OptimizePhysicsSimulation()
{
    // Apply physics optimization based on quality level
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("p.MaxSubsteps %d"), MaxPhysicsSubsteps));
    
    // Adjust physics simulation distance
    float PhysicsDistance = FMath::Lerp(500.0f, 1500.0f, CurrentQualityLevel / 3.0f);
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("p.ChaosImmPhysicsMaxDistance %f"), PhysicsDistance));
    
    // Set collision complexity based on quality
    if (CurrentQualityLevel <= 1)
    {
        GetWorld()->Exec(GetWorld(), TEXT("p.DefaultCollisionComplexity Simple"));
    }
    else
    {
        GetWorld()->Exec(GetWorld(), TEXT("p.DefaultCollisionComplexity Complex"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Optimized physics simulation (Distance: %f)"), PhysicsDistance);
}

void UPerformanceManager::OptimizeMemoryStreaming()
{
    // Adjust texture streaming pool size based on available memory
    float StreamingPoolSize = bIsConsole ? 2048.0f : 4096.0f; // MB
    if (CurrentQualityLevel <= 1)
    {
        StreamingPoolSize *= 0.7f;
    }
    
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Streaming.PoolSize %d"), FMath::RoundToInt(StreamingPoolSize)));
    
    // Adjust mesh streaming settings
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Streaming.MipBias %d"), 3 - CurrentQualityLevel));
    
    // Set streaming distance multiplier
    float StreamingDistance = FMath::Lerp(0.5f, 1.2f, CurrentQualityLevel / 3.0f);
    GetWorld()->Exec(GetWorld(), *FString::Printf(TEXT("r.Streaming.MaxEffectiveScreenSize %f"), StreamingDistance));
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Optimized memory streaming (Pool: %f MB)"), StreamingPoolSize);
}