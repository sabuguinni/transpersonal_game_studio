// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformMemory.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Stats/StatsData.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

// Console variables for runtime performance tuning
static TAutoConsoleVariable<int32> CVarPerformanceQualityLevel(
    TEXT("r.JurassicQuality"),
    2,
    TEXT("Jurassic game quality level (0=Low, 1=Medium, 2=High, 3=Epic)"),
    ECVF_Scalability | ECVF_RenderThreadSafe
);

static TAutoConsoleVariable<bool> CVarDynamicQualityEnabled(
    TEXT("r.JurassicDynamicQuality"),
    true,
    TEXT("Enable dynamic quality scaling based on performance"),
    ECVF_Scalability
);

static TAutoConsoleVariable<float> CVarJurassicTargetFPS(
    TEXT("r.JurassicTargetFPS"),
    60.0f,
    TEXT("Target FPS for Jurassic game"),
    ECVF_Scalability
);

static TAutoConsoleVariable<int32> CVarMaxVisibleDinosaurs(
    TEXT("r.JurassicMaxDinosaurs"),
    50,
    TEXT("Maximum number of visible dinosaurs"),
    ECVF_Scalability
);

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentFPS = 0.0f;
    CurrentFrameTime = 0.0f;
    CurrentMemoryUsage = 0.0f;
    CurrentQualityLevel = 2;
    LastQualityCheckTime = 0.0f;
    
    bIsHighEndPC = false;
    bIsConsole = false;
    bSupportsDLSS = false;
    bSupportsRayTracing = false;
    
    RecentFrameTimes.Reserve(30); // Store 3 seconds of frame times at 10Hz
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerformanceSystem();
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdatePerformanceMetrics();
    
    if (bDynamicQualityEnabled)
    {
        CheckFrameTimeTarget();
        AdjustQualityIfNeeded();
    }
}

void UPerformanceManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void UPerformanceManager::InitializePerformanceSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Initializing Jurassic Performance System..."));
    
    // Detect platform capabilities
    DetectPlatformCapabilities();
    
    // Set initial quality level based on hardware
    int32 RecommendedQuality = GetRecommendedQualityLevel();
    SetQualityLevel(RecommendedQuality);
    
    // Apply Jurassic-specific optimizations
    ApplyJurassicOptimizations();
    
    UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Performance system initialized. Quality Level: %d, Platform: %s"), 
           CurrentQualityLevel, bIsConsole ? TEXT("Console") : TEXT("PC"));
}

FPerformanceMetrics UPerformanceManager::GetCurrentMetrics() const
{
    FPerformanceMetrics Metrics;
    Metrics.FPS = CurrentFPS;
    Metrics.FrameTime = CurrentFrameTime;
    Metrics.MemoryUsage = CurrentMemoryUsage;
    Metrics.QualityLevel = CurrentQualityLevel;
    Metrics.bMeetingTargetFPS = CurrentFPS >= (bIsConsole ? TargetFPS_Console : TargetFPS_PC) * 0.95f;
    
    // Get additional metrics
    Metrics.VisibleDinosaurs = FMath::Min(CVarMaxVisibleDinosaurs.GetValueOnGameThread(), MaxVisibleDinosaurs);
    Metrics.PhysicsLoad = CurrentFrameTime > 0 ? FMath::Clamp(CurrentFrameTime / (bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC), 0.0f, 2.0f) : 0.0f;
    
    return Metrics;
}

void UPerformanceManager::SetQualityLevel(int32 NewQualityLevel)
{
    NewQualityLevel = FMath::Clamp(NewQualityLevel, 0, 3);
    
    if (NewQualityLevel != CurrentQualityLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Changing quality level from %d to %d"), CurrentQualityLevel, NewQualityLevel);
        
        CurrentQualityLevel = NewQualityLevel;
        ApplyQualityLevel(NewQualityLevel);
        
        // Update console variable
        CVarPerformanceQualityLevel->Set(NewQualityLevel);
    }
}

void UPerformanceManager::SetDynamicQualityEnabled(bool bEnable)
{
    bDynamicQualityEnabled = bEnable;
    CVarDynamicQualityEnabled->Set(bEnable);
    
    UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Dynamic quality scaling %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

int32 UPerformanceManager::GetRecommendedQualityLevel() const
{
    if (bIsConsole)
    {
        // Console: Medium quality for stable 30fps
        return 1;
    }
    else if (bIsHighEndPC)
    {
        // High-end PC: High quality for 60fps
        return 2;
    }
    else
    {
        // Mid-range PC: Medium quality
        return 1;
    }
}

void UPerformanceManager::ApplyJurassicOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Applying Jurassic-specific optimizations..."));
    
    OptimizeDinosaurRendering();
    OptimizeVegetationSystem();
    OptimizePhysicsSimulation();
    OptimizeMemoryStreaming();
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current FPS and frame time
    CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Track recent frame times for averaging
    RecentFrameTimes.Add(CurrentFrameTime);
    if (RecentFrameTimes.Num() > 30)
    {
        RecentFrameTimes.RemoveAt(0);
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

void UPerformanceManager::CheckFrameTimeTarget()
{
    float TargetFrameTime = bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC;
    
    // Check if we're consistently over budget
    if (RecentFrameTimes.Num() >= 10)
    {
        float AverageFrameTime = 0.0f;
        for (int32 i = RecentFrameTimes.Num() - 10; i < RecentFrameTimes.Num(); i++)
        {
            AverageFrameTime += RecentFrameTimes[i];
        }
        AverageFrameTime /= 10.0f;
        
        if (AverageFrameTime > TargetFrameTime + QualityAdjustmentThreshold)
        {
            // Performance is poor, consider lowering quality
            if (CurrentQualityLevel > 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Frame time over budget (%.2fms > %.2fms), considering quality reduction"), 
                       AverageFrameTime, TargetFrameTime);
            }
        }
        else if (AverageFrameTime < TargetFrameTime - QualityAdjustmentThreshold)
        {
            // Performance is good, consider raising quality
            if (CurrentQualityLevel < 3)
            {
                UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Frame time under budget (%.2fms < %.2fms), considering quality increase"), 
                       AverageFrameTime, TargetFrameTime);
            }
        }
    }
}

void UPerformanceManager::CheckMemoryUsage()
{
    float MaxMemory = bIsConsole ? MaxMemoryUsage_Console : MaxMemoryUsage_PC;
    
    if (CurrentMemoryUsage > MaxMemory * 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Memory usage high: %.2fMB / %.2fMB"), 
               CurrentMemoryUsage, MaxMemory);
        
        // Force garbage collection
        GEngine->ForceGarbageCollection(true);
    }
}

void UPerformanceManager::AdjustQualityIfNeeded()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastQualityCheckTime < QualityCheckInterval)
    {
        return;
    }
    
    LastQualityCheckTime = CurrentTime;
    
    CheckMemoryUsage();
    
    // Calculate average frame time over the check interval
    if (RecentFrameTimes.Num() < 10)
    {
        return;
    }
    
    float AverageFrameTime = 0.0f;
    int32 SamplesToCheck = FMath::Min(20, RecentFrameTimes.Num());
    for (int32 i = RecentFrameTimes.Num() - SamplesToCheck; i < RecentFrameTimes.Num(); i++)
    {
        AverageFrameTime += RecentFrameTimes[i];
    }
    AverageFrameTime /= SamplesToCheck;
    
    float TargetFrameTime = bIsConsole ? FrameTimeBudget_Console : FrameTimeBudget_PC;
    
    // Adjust quality based on performance
    if (AverageFrameTime > TargetFrameTime + QualityAdjustmentThreshold * 2.0f && CurrentQualityLevel > 0)
    {
        // Significantly over budget, reduce quality
        SetQualityLevel(CurrentQualityLevel - 1);
    }
    else if (AverageFrameTime < TargetFrameTime - QualityAdjustmentThreshold && CurrentQualityLevel < 3)
    {
        // Well under budget, try increasing quality
        SetQualityLevel(CurrentQualityLevel + 1);
    }
}

void UPerformanceManager::DetectPlatformCapabilities()
{
    // Detect console platforms
    bIsConsole = PLATFORM_CONSOLE;
    
    if (!bIsConsole)
    {
        // Detect high-end PC capabilities
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        float TotalMemoryGB = MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f);
        
        // Consider high-end if we have 16GB+ RAM
        bIsHighEndPC = TotalMemoryGB >= 15.0f;
        
        // Check for ray tracing support
        bSupportsRayTracing = GRHISupportsRayTracing;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Platform Detection - Console: %s, High-end PC: %s, Ray Tracing: %s"), 
           bIsConsole ? TEXT("Yes") : TEXT("No"),
           bIsHighEndPC ? TEXT("Yes") : TEXT("No"),
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
    SetRenderingQuality(QualityLevel);
    SetPhysicsQuality(QualityLevel);
    SetAudioQuality(QualityLevel);
}

void UPerformanceManager::SetRenderingQuality(int32 Level)
{
    // Apply scalability settings based on quality level
    static IConsoleVariable* CVarSGResolutionQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ResolutionQuality"));
    static IConsoleVariable* CVarSGViewDistanceQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ViewDistanceQuality"));
    static IConsoleVariable* CVarSGShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    static IConsoleVariable* CVarSGTextureQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.TextureQuality"));
    static IConsoleVariable* CVarSGEffectsQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.EffectsQuality"));
    static IConsoleVariable* CVarSGPostProcessQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.PostProcessQuality"));
    
    if (CVarSGResolutionQuality) CVarSGResolutionQuality->Set(Level);
    if (CVarSGViewDistanceQuality) CVarSGViewDistanceQuality->Set(Level);
    if (CVarSGShadowQuality) CVarSGShadowQuality->Set(Level);
    if (CVarSGTextureQuality) CVarSGTextureQuality->Set(Level);
    if (CVarSGEffectsQuality) CVarSGEffectsQuality->Set(Level);
    if (CVarSGPostProcessQuality) CVarSGPostProcessQuality->Set(Level);
}

void UPerformanceManager::SetPhysicsQuality(int32 Level)
{
    // Adjust physics simulation quality
    MaxPhysicsSubsteps = FMath::Clamp(4 - Level, 1, 6);
    
    static IConsoleVariable* CVarPhysicsSubsteps = IConsoleManager::Get().FindConsoleVariable(TEXT("p.MaxSubsteps"));
    if (CVarPhysicsSubsteps)
    {
        CVarPhysicsSubsteps->Set(MaxPhysicsSubsteps);
    }
}

void UPerformanceManager::SetAudioQuality(int32 Level)
{
    // Adjust audio quality settings
    static IConsoleVariable* CVarAudioQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("au.StreamingLookahead"));
    if (CVarAudioQuality)
    {
        float LookaheadTime = FMath::Lerp(1.0f, 4.0f, Level / 3.0f);
        CVarAudioQuality->Set(LookaheadTime);
    }
}

void UPerformanceManager::OptimizeDinosaurRendering()
{
    // Set dinosaur-specific rendering optimizations
    MaxVisibleDinosaurs = FMath::Clamp(50 - (3 - CurrentQualityLevel) * 10, 20, 100);
    DinosaurCullDistance = FMath::Clamp(2000.0f + CurrentQualityLevel * 500.0f, 1000.0f, 4000.0f);
    
    CVarMaxVisibleDinosaurs->Set(MaxVisibleDinosaurs);
    
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Dinosaur rendering optimized - Max visible: %d, Cull distance: %.0fm"), 
           MaxVisibleDinosaurs, DinosaurCullDistance);
}

void UPerformanceManager::OptimizeVegetationSystem()
{
    // Adjust vegetation density based on quality level
    VegetationDensityScale = FMath::Clamp(0.5f + CurrentQualityLevel * 0.25f, 0.3f, 1.5f);
    
    static IConsoleVariable* CVarFoliageDensity = IConsoleManager::Get().FindConsoleVariable(TEXT("foliage.DensityScale"));
    if (CVarFoliageDensity)
    {
        CVarFoliageDensity->Set(VegetationDensityScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Vegetation optimized - Density scale: %.2f"), VegetationDensityScale);
}

void UPerformanceManager::OptimizePhysicsSimulation()
{
    // Physics optimization based on quality level
    static IConsoleVariable* CVarPhysicsSubstepping = IConsoleManager::Get().FindConsoleVariable(TEXT("p.EnableSubstepping"));
    static IConsoleVariable* CVarChaosThreads = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.Solver.AsyncDt"));
    
    if (CVarPhysicsSubstepping)
    {
        CVarPhysicsSubstepping->Set(CurrentQualityLevel >= 2 ? 1 : 0);
    }
    
    if (CVarChaosThreads)
    {
        float AsyncDt = CurrentQualityLevel >= 2 ? 0.016667f : 0.033333f; // 60fps vs 30fps physics
        CVarChaosThreads->Set(AsyncDt);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Physics optimized - Substeps: %d"), MaxPhysicsSubsteps);
}

void UPerformanceManager::OptimizeMemoryStreaming()
{
    // Texture streaming optimization
    static IConsoleVariable* CVarTexturePoolSize = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"));
    static IConsoleVariable* CVarTextureLODBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.MipBias"));
    
    if (CVarTexturePoolSize)
    {
        int32 PoolSize = bIsConsole ? 1000 : (bIsHighEndPC ? 2000 : 1500);
        CVarTexturePoolSize->Set(PoolSize);
    }
    
    if (CVarTextureLODBias)
    {
        float LODBias = FMath::Clamp(1.0f - CurrentQualityLevel * 0.33f, 0.0f, 1.0f);
        CVarTextureLODBias->Set(LODBias);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Memory streaming optimized for quality level %d"), CurrentQualityLevel);
}