#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Console.h"
#include "UnrealEngine.h"

APerformanceManager::APerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    CurrentSettings = FPerformanceSettings();
    FPSHistory.Reserve(MaxFPSHistorySize);
}

void APerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Starting performance monitoring"));
    
    // Apply initial settings based on target
    ApplyTargetSettings();
    
    if (bAutoOptimizationEnabled)
    {
        StartAutoOptimization();
    }
}

void APerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update FPS tracking
    UpdateFPSHistory();
    
    // Check if we need to optimize
    if (bAutoOptimizationEnabled)
    {
        LastOptimizationCheck += DeltaTime;
        if (LastOptimizationCheck >= OptimizationCheckInterval)
        {
            CheckPerformanceAndOptimize();
            LastOptimizationCheck = 0.0f;
        }
    }
}

float APerformanceManager::GetCurrentFPS() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

float APerformanceManager::GetAverageFPS() const
{
    if (FPSHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float FPS : FPSHistory)
    {
        Total += FPS;
    }
    
    return Total / FPSHistory.Num();
}

float APerformanceManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

int32 APerformanceManager::GetDrawCalls() const
{
    // This would require access to rendering stats
    // For now, return a placeholder
    return 0;
}

int32 APerformanceManager::GetTriangleCount() const
{
    // This would require access to rendering stats
    // For now, return a placeholder
    return 0;
}

void APerformanceManager::SetPerformanceTarget(EPerformanceTarget Target)
{
    CurrentTarget = Target;
    ApplyTargetSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Target set to %d"), (int32)Target);
}

void APerformanceManager::SetPerformanceLevel(EPerformanceLevel Level)
{
    CurrentLevel = Level;
    CurrentSettings = GetSettingsForLevel(Level);
    ApplyPerformanceSettings(CurrentSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance level set to %d"), (int32)Level);
}

void APerformanceManager::ApplyPerformanceSettings(const FPerformanceSettings& Settings)
{
    CurrentSettings = Settings;
    
    // Apply shadow settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Shadow.MaxResolution %d"), Settings.ShadowMapResolution));
    
    // Apply view distance
    ExecuteConsoleCommand(FString::Printf(TEXT("r.ViewDistanceScale %f"), Settings.ViewDistanceScale));
    
    // Apply lighting settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.MaxAnisotropy %d"), FMath::Clamp(Settings.MaxLights, 1, 16)));
    
    // Apply ray tracing
    ExecuteConsoleCommand(FString::Printf(TEXT("r.RayTracing %d"), Settings.bEnableRayTracing ? 1 : 0));
    
    // Apply Lumen
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Lumen.GlobalIllumination %d"), Settings.bEnableLumen ? 1 : 0));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Lumen.Reflections %d"), Settings.bEnableLumen ? 1 : 0));
    
    // Apply anti-aliasing
    ExecuteConsoleCommand(FString::Printf(TEXT("r.DefaultFeature.AntiAliasing %d"), Settings.AntiAliasingMethod));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.PostProcessAAQuality %d"), Settings.PostProcessQuality));
    
    // Apply texture settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Streaming.PoolSize %d"), Settings.TexturePoolSize));
    
    // Apply LOD settings
    ExecuteConsoleCommand(FString::Printf(TEXT("r.StaticMeshLODDistanceScale %f"), 1.0f + Settings.LODBias));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Applied performance settings"));
}

void APerformanceManager::OptimizeShadows()
{
    // Reduce shadow resolution and distance for performance
    ExecuteConsoleCommand(TEXT("r.Shadow.MaxResolution 1024"));
    ExecuteConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.05"));
    ExecuteConsoleCommand(TEXT("r.Shadow.DistanceScale 0.5"));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Shadows optimized"));
}

void APerformanceManager::OptimizeLighting()
{
    // Optimize lighting for performance
    ExecuteConsoleCommand(TEXT("r.LightMaxDrawDistanceScale 0.8"));
    ExecuteConsoleCommand(TEXT("r.MaxAnisotropy 8"));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Lighting optimized"));
}

void APerformanceManager::OptimizeTextures()
{
    // Optimize texture streaming
    ExecuteConsoleCommand(TEXT("r.Streaming.PoolSize 1500"));
    ExecuteConsoleCommand(TEXT("r.Streaming.MaxTempMemoryAllowed 50"));
    ExecuteConsoleCommand(TEXT("r.MipMapLODBias 1"));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Textures optimized"));
}

void APerformanceManager::OptimizeLOD()
{
    // Increase LOD bias for better performance
    ExecuteConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.5"));
    ExecuteConsoleCommand(TEXT("r.SkeletalMeshLODBias 1"));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: LOD optimized"));
}

void APerformanceManager::EnablePerformanceMode()
{
    // Apply aggressive performance optimizations
    OptimizeShadows();
    OptimizeLighting();
    OptimizeTextures();
    OptimizeLOD();
    
    // Disable expensive features
    ExecuteConsoleCommand(TEXT("r.RayTracing 0"));
    ExecuteConsoleCommand(TEXT("r.Lumen.GlobalIllumination.Quality 1"));
    ExecuteConsoleCommand(TEXT("r.PostProcessAAQuality 2"));
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance mode enabled"));
}

void APerformanceManager::DisablePerformanceMode()
{
    // Restore quality settings
    ApplyTargetSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance mode disabled"));
}

void APerformanceManager::StartAutoOptimization()
{
    bAutoOptimizationEnabled = true;
    LastOptimizationCheck = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Auto-optimization started"));
}

void APerformanceManager::StopAutoOptimization()
{
    bAutoOptimizationEnabled = false;
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Auto-optimization stopped"));
}

void APerformanceManager::UpdateFPSHistory()
{
    float CurrentFPS = GetCurrentFPS();
    
    FPSHistory.Add(CurrentFPS);
    
    // Keep only recent history
    if (FPSHistory.Num() > MaxFPSHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }
}

void APerformanceManager::CheckPerformanceAndOptimize()
{
    float AvgFPS = GetAverageFPS();
    
    if (AvgFPS < MinAcceptableFPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: FPS below threshold (%.1f), applying optimizations"), AvgFPS);
        
        // Apply progressive optimizations based on how bad performance is
        if (AvgFPS < MinAcceptableFPS * 0.5f)
        {
            // Very bad performance - aggressive optimization
            EnablePerformanceMode();
        }
        else if (AvgFPS < MinAcceptableFPS * 0.75f)
        {
            // Moderate performance issues
            OptimizeShadows();
            OptimizeLOD();
        }
        else
        {
            // Minor performance issues
            OptimizeTextures();
        }
    }
    else if (AvgFPS > TargetFPS * 1.2f)
    {
        // Performance is good, we can increase quality if needed
        UE_LOG(LogTemp, Log, TEXT("PerformanceManager: Performance is good (%.1f FPS)"), AvgFPS);
    }
}

void APerformanceManager::ApplyTargetSettings()
{
    FPerformanceSettings TargetSettings = GetSettingsForTarget(CurrentTarget);
    ApplyPerformanceSettings(TargetSettings);
    
    // Set target FPS based on platform
    switch (CurrentTarget)
    {
        case EPerformanceTarget::PC_High:
            TargetFPS = 60.0f;
            MinAcceptableFPS = 45.0f;
            break;
        case EPerformanceTarget::PC_Medium:
            TargetFPS = 45.0f;
            MinAcceptableFPS = 30.0f;
            break;
        case EPerformanceTarget::Console:
            TargetFPS = 30.0f;
            MinAcceptableFPS = 25.0f;
            break;
        case EPerformanceTarget::Mobile:
            TargetFPS = 30.0f;
            MinAcceptableFPS = 20.0f;
            break;
    }
    
    ExecuteConsoleCommand(FString::Printf(TEXT("t.MaxFPS %f"), TargetFPS));
}

void APerformanceManager::ExecuteConsoleCommand(const FString& Command)
{
    if (GEngine && GEngine->GetGameViewport())
    {
        GEngine->GetGameViewport()->ConsoleCommand(Command);
    }
}

FPerformanceSettings APerformanceManager::GetSettingsForLevel(EPerformanceLevel Level) const
{
    FPerformanceSettings Settings;
    
    switch (Level)
    {
        case EPerformanceLevel::Ultra:
            Settings.ShadowMapResolution = 4096;
            Settings.ViewDistanceScale = 1.2f;
            Settings.MaxLights = 32;
            Settings.bEnableRayTracing = true;
            Settings.bEnableLumen = true;
            Settings.AntiAliasingMethod = 2; // TAA
            Settings.PostProcessQuality = 4;
            Settings.TexturePoolSize = 3000;
            Settings.LODBias = -0.5f;
            Settings.CullingDistance = 15000.0f;
            break;
            
        case EPerformanceLevel::High:
            Settings.ShadowMapResolution = 2048;
            Settings.ViewDistanceScale = 1.0f;
            Settings.MaxLights = 16;
            Settings.bEnableRayTracing = false;
            Settings.bEnableLumen = true;
            Settings.AntiAliasingMethod = 2; // TAA
            Settings.PostProcessQuality = 3;
            Settings.TexturePoolSize = 2000;
            Settings.LODBias = 0.0f;
            Settings.CullingDistance = 10000.0f;
            break;
            
        case EPerformanceLevel::Medium:
            Settings.ShadowMapResolution = 1024;
            Settings.ViewDistanceScale = 0.8f;
            Settings.MaxLights = 8;
            Settings.bEnableRayTracing = false;
            Settings.bEnableLumen = true;
            Settings.AntiAliasingMethod = 1; // FXAA
            Settings.PostProcessQuality = 2;
            Settings.TexturePoolSize = 1500;
            Settings.LODBias = 0.5f;
            Settings.CullingDistance = 8000.0f;
            break;
            
        case EPerformanceLevel::Low:
            Settings.ShadowMapResolution = 512;
            Settings.ViewDistanceScale = 0.6f;
            Settings.MaxLights = 4;
            Settings.bEnableRayTracing = false;
            Settings.bEnableLumen = false;
            Settings.AntiAliasingMethod = 1; // FXAA
            Settings.PostProcessQuality = 1;
            Settings.TexturePoolSize = 1000;
            Settings.LODBias = 1.0f;
            Settings.CullingDistance = 5000.0f;
            break;
            
        case EPerformanceLevel::Potato:
            Settings.ShadowMapResolution = 256;
            Settings.ViewDistanceScale = 0.4f;
            Settings.MaxLights = 2;
            Settings.bEnableRayTracing = false;
            Settings.bEnableLumen = false;
            Settings.AntiAliasingMethod = 0; // None
            Settings.PostProcessQuality = 0;
            Settings.TexturePoolSize = 500;
            Settings.LODBias = 2.0f;
            Settings.CullingDistance = 3000.0f;
            break;
    }
    
    return Settings;
}

FPerformanceSettings APerformanceManager::GetSettingsForTarget(EPerformanceTarget Target) const
{
    switch (Target)
    {
        case EPerformanceTarget::PC_High:
            return GetSettingsForLevel(EPerformanceLevel::High);
        case EPerformanceTarget::PC_Medium:
            return GetSettingsForLevel(EPerformanceLevel::Medium);
        case EPerformanceTarget::Console:
            return GetSettingsForLevel(EPerformanceLevel::Medium);
        case EPerformanceTarget::Mobile:
            return GetSettingsForLevel(EPerformanceLevel::Low);
        default:
            return GetSettingsForLevel(EPerformanceLevel::Medium);
    }
}