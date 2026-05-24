#include "Perf_PerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/GameUserSettings.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"

APerf_PerformanceManager::APerf_PerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    TargetPerformanceLevel = EPerf_PerformanceLevel::High;
    OptimizationMode = EPerf_OptimizationMode::Conservative;
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    bAutoOptimize = true;
    OptimizationCheckInterval = 1.0f;
    LastOptimizationCheck = 0.0f;
    MaxFPSHistorySize = 60;

    // Initialize optimization settings
    OptimizationSettings = FPerf_OptimizationSettings();
}

void APerf_PerformanceManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Performance Manager initialized"));
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
    
    // Start performance monitoring
    FPSHistory.Reserve(MaxFPSHistorySize);
}

void APerf_PerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance metrics
    UpdatePerformanceMetrics();

    // Check if we need to perform auto-optimization
    if (bAutoOptimize)
    {
        LastOptimizationCheck += DeltaTime;
        if (LastOptimizationCheck >= OptimizationCheckInterval)
        {
            PerformAutoOptimization();
            LastOptimizationCheck = 0.0f;
        }
    }
}

void APerf_PerformanceManager::UpdatePerformanceMetrics()
{
    // Get current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    }

    // Update FPS history
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > MaxFPSHistorySize)
    {
        FPSHistory.RemoveAt(0);
    }

    // Calculate average FPS
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
        }
        CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
    }

    // Get render stats (simplified - in real implementation would use proper stat system)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f; // Approximate
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.8f; // Approximate
    CurrentMetrics.GPUTime = CurrentMetrics.FrameTime * 0.9f; // Approximate

    // Estimate draw calls and triangles based on scene complexity
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), AllActors);
        CurrentMetrics.DrawCalls = AllActors.Num() * 2; // Rough estimate
        CurrentMetrics.Triangles = AllActors.Num() * 1000; // Rough estimate
    }

    // Memory usage (simplified)
    CurrentMetrics.UsedMemoryMB = 2048.0f + (CurrentMetrics.Triangles / 1000.0f);
    CurrentMetrics.AvailableMemoryMB = 8192.0f - CurrentMetrics.UsedMemoryMB;
}

void APerf_PerformanceManager::ApplyOptimizationSettings()
{
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (!UserSettings)
    {
        return;
    }

    // Apply LOD settings
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ForceLOD %d"), 
            OptimizationSettings.LODBias > 0 ? 1 : -1));
    }

    // Apply culling settings
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.HZBOcclusion %d"), 
            OptimizationSettings.bEnableOcclusionCulling ? 1 : 0));
        
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.CullDistanceScale %f"), 
            OptimizationSettings.CullDistance / 10000.0f));
    }

    // Apply shadow settings
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.MaxResolution %d"), 
            OptimizationSettings.TextureQuality * 512));
        
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.Shadow.DistanceScale %f"), 
            OptimizationSettings.ShadowDistance / 3000.0f));
    }

    // Apply texture quality
    UserSettings->SetTextureQuality(OptimizationSettings.TextureQuality);
    
    // Apply effects quality
    UserSettings->SetVisualEffectQuality(OptimizationSettings.EffectsQuality);
    
    // Apply post-process quality
    UserSettings->SetPostProcessingQuality(OptimizationSettings.PostProcessQuality);

    // Apply settings
    UserSettings->ApplySettings(false);

    UE_LOG(LogTemp, Warning, TEXT("Applied optimization settings"));
}

void APerf_PerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    TargetPerformanceLevel = NewLevel;

    // Adjust settings based on performance level
    switch (NewLevel)
    {
        case EPerf_PerformanceLevel::Low:
            OptimizationSettings.LODBias = 2.0f;
            OptimizationSettings.CullDistance = 5000.0f;
            OptimizationSettings.ShadowDistance = 1500.0f;
            OptimizationSettings.TextureQuality = 1;
            OptimizationSettings.EffectsQuality = 1;
            OptimizationSettings.PostProcessQuality = 1;
            TargetFPS = 30.0f;
            break;

        case EPerf_PerformanceLevel::Medium:
            OptimizationSettings.LODBias = 1.0f;
            OptimizationSettings.CullDistance = 7500.0f;
            OptimizationSettings.ShadowDistance = 2250.0f;
            OptimizationSettings.TextureQuality = 2;
            OptimizationSettings.EffectsQuality = 2;
            OptimizationSettings.PostProcessQuality = 2;
            TargetFPS = 45.0f;
            break;

        case EPerf_PerformanceLevel::High:
            OptimizationSettings.LODBias = 0.0f;
            OptimizationSettings.CullDistance = 10000.0f;
            OptimizationSettings.ShadowDistance = 3000.0f;
            OptimizationSettings.TextureQuality = 3;
            OptimizationSettings.EffectsQuality = 3;
            OptimizationSettings.PostProcessQuality = 3;
            TargetFPS = 60.0f;
            break;

        case EPerf_PerformanceLevel::Ultra:
            OptimizationSettings.LODBias = -1.0f;
            OptimizationSettings.CullDistance = 15000.0f;
            OptimizationSettings.ShadowDistance = 5000.0f;
            OptimizationSettings.TextureQuality = 4;
            OptimizationSettings.EffectsQuality = 4;
            OptimizationSettings.PostProcessQuality = 4;
            TargetFPS = 120.0f;
            break;
    }

    ApplyOptimizationSettings();
}

void APerf_PerformanceManager::SetOptimizationMode(EPerf_OptimizationMode NewMode)
{
    OptimizationMode = NewMode;
    
    switch (NewMode)
    {
        case EPerf_OptimizationMode::Disabled:
            bAutoOptimize = false;
            break;
            
        case EPerf_OptimizationMode::Conservative:
            bAutoOptimize = true;
            OptimizationCheckInterval = 2.0f;
            break;
            
        case EPerf_OptimizationMode::Aggressive:
            bAutoOptimize = true;
            OptimizationCheckInterval = 1.0f;
            break;
            
        case EPerf_OptimizationMode::Extreme:
            bAutoOptimize = true;
            OptimizationCheckInterval = 0.5f;
            break;
    }
}

float APerf_PerformanceManager::GetAverageFPS() const
{
    return CurrentMetrics.AverageFPS;
}

bool APerf_PerformanceManager::IsPerformanceAcceptable() const
{
    return CurrentMetrics.AverageFPS >= MinimumFPS;
}

void APerf_PerformanceManager::ForceOptimization()
{
    PerformAutoOptimization();
}

void APerf_PerformanceManager::ResetToDefaultSettings()
{
    OptimizationSettings = FPerf_OptimizationSettings();
    SetPerformanceLevel(EPerf_PerformanceLevel::High);
    ApplyOptimizationSettings();
}

void APerf_PerformanceManager::PerformAutoOptimization()
{
    if (!IsPerformanceAcceptable())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f FPS), applying optimizations"), 
               CurrentMetrics.AverageFPS);

        // Gradually reduce quality settings
        if (OptimizationMode == EPerf_OptimizationMode::Conservative)
        {
            if (OptimizationSettings.EffectsQuality > 1)
            {
                OptimizationSettings.EffectsQuality--;
            }
            else if (OptimizationSettings.ShadowDistance > 1000.0f)
            {
                OptimizationSettings.ShadowDistance *= 0.8f;
            }
        }
        else if (OptimizationMode == EPerf_OptimizationMode::Aggressive)
        {
            OptimizationSettings.LODBias += 0.5f;
            OptimizationSettings.CullDistance *= 0.9f;
            OptimizationSettings.ShadowDistance *= 0.8f;
            
            if (OptimizationSettings.TextureQuality > 1)
            {
                OptimizationSettings.TextureQuality--;
            }
        }
        else if (OptimizationMode == EPerf_OptimizationMode::Extreme)
        {
            OptimizationSettings.LODBias += 1.0f;
            OptimizationSettings.CullDistance *= 0.7f;
            OptimizationSettings.ShadowDistance *= 0.6f;
            
            OptimizationSettings.TextureQuality = FMath::Max(1, OptimizationSettings.TextureQuality - 1);
            OptimizationSettings.EffectsQuality = FMath::Max(1, OptimizationSettings.EffectsQuality - 1);
            OptimizationSettings.PostProcessQuality = FMath::Max(1, OptimizationSettings.PostProcessQuality - 1);
        }

        ApplyOptimizationSettings();
    }
    else if (CurrentMetrics.AverageFPS > TargetFPS * 1.2f)
    {
        // Performance is good, we can increase quality slightly
        if (OptimizationMode != EPerf_OptimizationMode::Disabled)
        {
            if (OptimizationSettings.TextureQuality < 4)
            {
                OptimizationSettings.TextureQuality++;
                ApplyOptimizationSettings();
            }
        }
    }
}

void APerf_PerformanceManager::ApplyLODOptimizations()
{
    // Apply LOD optimizations to all static mesh actors
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> StaticMeshActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);

    for (AActor* Actor : StaticMeshActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetForcedLodModel(OptimizationSettings.LODBias > 0 ? 2 : 0);
            }
        }
    }
}

void APerf_PerformanceManager::ApplyCullingOptimizations()
{
    // Apply culling optimizations
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.CullDistanceScale %f"), 
            OptimizationSettings.CullDistance / 10000.0f));
    }
}

void APerf_PerformanceManager::ApplyRenderingOptimizations()
{
    // Apply rendering optimizations
    if (GEngine)
    {
        // Adjust screen percentage for performance
        float ScreenPercentage = 100.0f;
        if (CurrentMetrics.AverageFPS < MinimumFPS)
        {
            ScreenPercentage = FMath::Clamp(CurrentMetrics.AverageFPS / MinimumFPS * 100.0f, 50.0f, 100.0f);
        }
        
        GEngine->Exec(GetWorld(), *FString::Printf(TEXT("r.ScreenPercentage %f"), ScreenPercentage));
    }
}

void APerf_PerformanceManager::ApplyMemoryOptimizations()
{
    // Apply memory optimizations
    if (CurrentMetrics.UsedMemoryMB > CurrentMetrics.AvailableMemoryMB * 0.8f)
    {
        // Force garbage collection
        if (GEngine)
        {
            GEngine->ForceGarbageCollection(true);
        }
        
        // Reduce texture streaming pool if needed
        if (GEngine)
        {
            GEngine->Exec(GetWorld(), TEXT("r.Streaming.PoolSize 512"));
        }
    }
}

void APerf_PerformanceManager::CollectPerformanceData()
{
    // Collect detailed performance data for analysis
    // This would be expanded in a real implementation to gather
    // more detailed profiling information
}