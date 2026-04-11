#include "PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/LightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/PostProcessVolume.h"

UPerformanceManager::UPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = MonitoringInterval;
    
    // Initialize default thresholds
    Thresholds = FPerformanceThresholds();
    CurrentMetrics = FPerformanceMetrics();
    
    // Initialize performance history
    for (int32 i = 0; i < 10; i++)
    {
        PerformanceHistory[i] = 60.0f; // Start optimistic
    }
}

void UPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Manager initialized - Target FPS: %.1f"), Thresholds.TargetFPS_PC);
    
    // Set initial quality based on platform
    #if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
        CurrentQualityScale = 1.0f; // Full quality on PC
    #else
        CurrentQualityScale = 0.8f; // Reduced quality on console
    #endif
}

void UPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        CheckPerformanceThresholds();
        
        if (bAutoOptimizationEnabled && !bOptimizationInProgress)
        {
            ApplyPerformanceOptimizations();
        }
        
        TimeSinceLastUpdate = 0.0f;
    }
    
    // Smooth quality transitions
    if (FMath::Abs(CurrentQualityScale - TargetQualityScale) > 0.01f)
    {
        CurrentQualityScale = FMath::FInterpTo(CurrentQualityScale, TargetQualityScale, DeltaTime, QualityTransitionSpeed);
        AdjustRenderQuality(CurrentQualityScale);
    }
}

void UPerformanceManager::UpdatePerformanceMetrics()
{
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        CurrentMetrics.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
    
    // Update FPS history for rolling average
    PerformanceHistory[HistoryIndex] = CurrentMetrics.CurrentFPS;
    HistoryIndex = (HistoryIndex + 1) % 10;
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count actors in world
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
        
        // Count visible actors (simplified - would need proper frustum culling check)
        int32 VisibleCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsHidden())
            {
                VisibleCount++;
            }
        }
        CurrentMetrics.VisibleActors = VisibleCount;
    }
    
    // Estimate draw calls and triangles (simplified)
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleActors; // Rough estimate
    CurrentMetrics.TriangleCount = CurrentMetrics.VisibleActors * 1000; // Very rough estimate
}

void UPerformanceManager::CheckPerformanceThresholds()
{
    // Calculate rolling average FPS
    float AverageFPS = 0.0f;
    for (int32 i = 0; i < 10; i++)
    {
        AverageFPS += PerformanceHistory[i];
    }
    AverageFPS /= 10.0f;
    
    // Check FPS threshold
    float TargetFPS = Thresholds.TargetFPS_PC;
    #if !PLATFORM_WINDOWS && !PLATFORM_MAC && !PLATFORM_LINUX
        TargetFPS = Thresholds.TargetFPS_Console;
    #endif
    
    if (AverageFPS < TargetFPS * 0.9f) // 10% tolerance
    {
        OnFPSThresholdExceeded.Broadcast(AverageFPS);
        UE_LOG(LogTemp, Warning, TEXT("Performance: FPS below target (%.1f < %.1f)"), AverageFPS, TargetFPS);
    }
    
    // Check memory threshold
    float MaxMemory = Thresholds.MaxMemoryUsage_PC;
    #if !PLATFORM_WINDOWS && !PLATFORM_MAC && !PLATFORM_LINUX
        MaxMemory = Thresholds.MaxMemoryUsage_Console;
    #endif
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemory * 0.9f) // 10% tolerance
    {
        OnMemoryThresholdExceeded.Broadcast(CurrentMetrics.MemoryUsageMB);
        UE_LOG(LogTemp, Warning, TEXT("Performance: Memory usage high (%.1f MB > %.1f MB)"), CurrentMetrics.MemoryUsageMB, MaxMemory);
    }
}

void UPerformanceManager::ApplyPerformanceOptimizations()
{
    if (bOptimizationInProgress) return;
    
    bOptimizationInProgress = true;
    
    // Calculate performance deficit
    float TargetFPS = Thresholds.TargetFPS_PC;
    #if !PLATFORM_WINDOWS && !PLATFORM_MAC && !PLATFORM_LINUX
        TargetFPS = Thresholds.TargetFPS_Console;
    #endif
    
    float AverageFPS = 0.0f;
    for (int32 i = 0; i < 10; i++)
    {
        AverageFPS += PerformanceHistory[i];
    }
    AverageFPS /= 10.0f;
    
    if (AverageFPS < TargetFPS * 0.9f)
    {
        // Performance is below target - reduce quality
        float PerformanceRatio = AverageFPS / TargetFPS;
        TargetQualityScale = FMath::Clamp(PerformanceRatio, 0.5f, 1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Performance: Reducing quality scale to %.2f (FPS: %.1f/%.1f)"), TargetQualityScale, AverageFPS, TargetFPS);
        
        // Apply specific optimizations
        OptimizeLODForCurrentPerformance();
        OptimizeLighting();
        OptimizeParticles();
    }
    else if (AverageFPS > TargetFPS * 1.1f && TargetQualityScale < 1.0f)
    {
        // Performance is above target - can increase quality
        TargetQualityScale = FMath::Min(TargetQualityScale + 0.1f, 1.0f);
        
        UE_LOG(LogTemp, Log, TEXT("Performance: Increasing quality scale to %.2f"), TargetQualityScale);
    }
    
    bOptimizationInProgress = false;
}

void UPerformanceManager::SetPerformanceThresholds(const FPerformanceThresholds& NewThresholds)
{
    Thresholds = NewThresholds;
    UE_LOG(LogTemp, Log, TEXT("Performance thresholds updated"));
}

bool UPerformanceManager::IsPerformanceWithinTargets() const
{
    float TargetFPS = Thresholds.TargetFPS_PC;
    #if !PLATFORM_WINDOWS && !PLATFORM_MAC && !PLATFORM_LINUX
        TargetFPS = Thresholds.TargetFPS_Console;
    #endif
    
    return CurrentMetrics.CurrentFPS >= TargetFPS * 0.9f;
}

void UPerformanceManager::ForcePerformanceOptimization()
{
    bOptimizationInProgress = false; // Reset flag
    ApplyPerformanceOptimizations();
}

void UPerformanceManager::SetGlobalLODBias(float LODBias)
{
    if (GEngine)
    {
        // Apply LOD bias to all static meshes
        UWorld* World = GetWorld();
        if (World)
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor)
                {
                    TArray<UStaticMeshComponent*> StaticMeshComponents;
                    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
                    
                    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
                    {
                        if (MeshComp)
                        {
                            MeshComp->SetForcedLodModel(FMath::Clamp(FMath::RoundToInt(LODBias), 0, 3));
                        }
                    }
                }
            }
        }
    }
}

void UPerformanceManager::OptimizeLODForCurrentPerformance()
{
    float LODBias = FMath::Lerp(0.0f, 2.0f, 1.0f - CurrentQualityScale);
    SetGlobalLODBias(LODBias);
}

void UPerformanceManager::TriggerGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Log, TEXT("Performance: Forced garbage collection"));
}

void UPerformanceManager::OptimizeMemoryUsage()
{
    TriggerGarbageCollection();
    
    // Additional memory optimizations could be added here
    // - Texture streaming optimization
    // - Audio memory cleanup
    // - Unused asset cleanup
}

void UPerformanceManager::AdjustRenderQuality(float QualityScale)
{
    if (GEngine && GEngine->GameViewport)
    {
        // Adjust screen percentage
        float ScreenPercentage = FMath::Lerp(50.0f, 100.0f, QualityScale);
        
        // Apply via console command (simplified approach)
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %f"), ScreenPercentage));
        
        // Adjust shadow quality
        int32 ShadowQuality = FMath::Clamp(FMath::RoundToInt(QualityScale * 3), 0, 3);
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ShadowQuality %d"), ShadowQuality));
        
        // Adjust post-processing quality
        int32 PostProcessQuality = FMath::Clamp(FMath::RoundToInt(QualityScale * 3), 0, 3);
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.PostProcessAAQuality %d"), PostProcessQuality));
    }
}

void UPerformanceManager::SetDynamicQualityEnabled(bool bEnabled)
{
    bDynamicQualityEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Dynamic quality adjustment %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerformanceManager::OptimizeLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Reduce light intensity and range for performance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<ULightComponent*> LightComponents;
            Actor->GetComponents<ULightComponent>(LightComponents);
            
            for (ULightComponent* LightComp : LightComponents)
            {
                if (LightComp)
                {
                    // Reduce light range for performance
                    float OriginalRadius = LightComp->AttenuationRadius;
                    float OptimizedRadius = OriginalRadius * CurrentQualityScale;
                    LightComp->SetAttenuationRadius(OptimizedRadius);
                }
            }
        }
    }
}

void UPerformanceManager::OptimizeParticles()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Reduce particle counts and complexity
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UParticleSystemComponent*> ParticleComponents;
            Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
            
            for (UParticleSystemComponent* ParticleComp : ParticleComponents)
            {
                if (ParticleComp)
                {
                    // Scale particle counts based on quality
                    float ParticleScale = FMath::Lerp(0.3f, 1.0f, CurrentQualityScale);
                    // Note: Actual particle scaling would require more complex implementation
                    // This is a simplified example
                }
            }
        }
    }
}

void UPerformanceManager::OptimizePostProcessing()
{
    // Adjust post-processing settings based on performance
    if (GEngine)
    {
        int32 PPQuality = FMath::Clamp(FMath::RoundToInt(CurrentQualityScale * 3), 0, 3);
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.PostProcessAAQuality %d"), PPQuality));
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.BloomQuality %d"), PPQuality));
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.MotionBlurQuality %d"), PPQuality));
    }
}

void UPerformanceManager::OptimizeShadows()
{
    if (GEngine)
    {
        // Adjust shadow quality and distance based on performance
        int32 ShadowQuality = FMath::Clamp(FMath::RoundToInt(CurrentQualityScale * 4), 1, 4);
        float ShadowDistance = FMath::Lerp(2000.0f, 8000.0f, CurrentQualityScale);
        
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ShadowQuality %d"), ShadowQuality));
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.MaxResolution %d"), ShadowQuality * 512));
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.DistanceScale %f"), CurrentQualityScale));
    }
}