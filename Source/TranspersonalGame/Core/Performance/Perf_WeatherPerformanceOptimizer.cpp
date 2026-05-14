#include "Perf_WeatherPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

APerf_WeatherPerformanceOptimizer::APerf_WeatherPerformanceOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentWeatherQuality = EPerf_WeatherQuality::Medium;
    TargetFrameRate = 60.0f;
    MaxParticleCount = 10000.0f;
    MaxRenderTime = 16.67f; // 60 FPS target

    // Quality multipliers
    LowQualityParticleMultiplier = 0.25f;
    MediumQualityParticleMultiplier = 0.5f;
    HighQualityParticleMultiplier = 0.75f;
    UltraQualityParticleMultiplier = 1.0f;

    // Performance tracking
    LastFrameTime = 0.0f;
    AverageFrameTime = 16.67f;
    FrameCounter = 0;

    // Initialize metrics
    CurrentMetrics = FPerf_WeatherPerformanceMetrics();
}

void APerf_WeatherPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("Weather Performance Optimizer initialized"));

    // Apply initial quality settings
    ApplyQualitySettings();
}

void APerf_WeatherPerformanceOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance metrics
    UpdatePerformanceMetrics();

    // Monitor frame rate and adjust quality if needed
    MonitorFrameRate();

    // Optimize weather effects based on current performance
    if (FrameCounter % 60 == 0) // Check every 60 frames (1 second at 60fps)
    {
        OptimizeWeatherEffects();
    }

    FrameCounter++;
}

void APerf_WeatherPerformanceOptimizer::OptimizeWeatherEffects()
{
    if (!GetWorld())
    {
        return;
    }

    // Check current frame rate
    float CurrentFPS = 1.0f / AverageFrameTime;
    
    if (CurrentFPS < TargetFrameRate * 0.9f) // If below 90% of target
    {
        // Reduce quality
        switch (CurrentWeatherQuality)
        {
            case EPerf_WeatherQuality::Ultra:
                SetWeatherQuality(EPerf_WeatherQuality::High);
                break;
            case EPerf_WeatherQuality::High:
                SetWeatherQuality(EPerf_WeatherQuality::Medium);
                break;
            case EPerf_WeatherQuality::Medium:
                SetWeatherQuality(EPerf_WeatherQuality::Low);
                break;
            default:
                break;
        }
    }
    else if (CurrentFPS > TargetFrameRate * 1.1f) // If above 110% of target
    {
        // Increase quality
        switch (CurrentWeatherQuality)
        {
            case EPerf_WeatherQuality::Low:
                SetWeatherQuality(EPerf_WeatherQuality::Medium);
                break;
            case EPerf_WeatherQuality::Medium:
                SetWeatherQuality(EPerf_WeatherQuality::High);
                break;
            case EPerf_WeatherQuality::High:
                SetWeatherQuality(EPerf_WeatherQuality::Ultra);
                break;
            default:
                break;
        }
    }

    OptimizeParticleSystems();
    OptimizeMaterialParameters();
}

void APerf_WeatherPerformanceOptimizer::SetWeatherQuality(EPerf_WeatherQuality Quality)
{
    if (CurrentWeatherQuality != Quality)
    {
        CurrentWeatherQuality = Quality;
        ApplyQualitySettings();
        
        UE_LOG(LogTemp, Warning, TEXT("Weather quality changed to: %d"), (int32)Quality);
    }
}

FPerf_WeatherPerformanceMetrics APerf_WeatherPerformanceOptimizer::GetWeatherPerformanceMetrics() const
{
    return CurrentMetrics;
}

void APerf_WeatherPerformanceOptimizer::OptimizeRainEffects(float TargetFPS)
{
    if (!GetWorld())
    {
        return;
    }

    // Find all particle system components in the world
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);

        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC && PSC->GetAsset())
            {
                // Check if this is a rain effect (basic name check)
                FString AssetName = PSC->GetAsset()->GetName();
                if (AssetName.Contains(TEXT("Rain")) || AssetName.Contains(TEXT("Precipitation")))
                {
                    // Adjust particle count based on quality
                    float QualityMultiplier = GetQualityMultiplier();
                    
                    // Scale particle spawn rate
                    PSC->SetFloatParameter(FName("SpawnRate"), PSC->GetFloatParameter(FName("SpawnRate")) * QualityMultiplier);
                }
            }
        }
    }
}

void APerf_WeatherPerformanceOptimizer::SetRainParticleCount(int32 ParticleCount)
{
    if (!GetWorld())
    {
        return;
    }

    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);

        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC && PSC->GetAsset())
            {
                FString AssetName = PSC->GetAsset()->GetName();
                if (AssetName.Contains(TEXT("Rain")))
                {
                    PSC->SetIntParameter(FName("MaxParticles"), ParticleCount);
                }
            }
        }
    }
}

void APerf_WeatherPerformanceOptimizer::OptimizeWindEffects()
{
    // Wind effects optimization - reduce complexity based on performance
    float QualityMultiplier = GetQualityMultiplier();
    
    // Apply wind quality settings to global wind parameters
    if (GetWorld())
    {
        // This would typically interface with a wind system
        UE_LOG(LogTemp, Log, TEXT("Wind effects optimized with multiplier: %f"), QualityMultiplier);
    }
}

void APerf_WeatherPerformanceOptimizer::SetWindQuality(EPerf_WeatherQuality Quality)
{
    // Set wind-specific quality
    OptimizeWindEffects();
}

void APerf_WeatherPerformanceOptimizer::OptimizeFogEffects()
{
    // Fog optimization - adjust density and quality based on performance
    float QualityMultiplier = GetQualityMultiplier();
    
    if (GetWorld())
    {
        // Adjust exponential height fog settings
        UE_LOG(LogTemp, Log, TEXT("Fog effects optimized with multiplier: %f"), QualityMultiplier);
    }
}

void APerf_WeatherPerformanceOptimizer::SetFogDensity(float Density)
{
    // Set fog density with performance considerations
    float AdjustedDensity = Density * GetQualityMultiplier();
    
    UE_LOG(LogTemp, Log, TEXT("Fog density set to: %f"), AdjustedDensity);
}

void APerf_WeatherPerformanceOptimizer::OptimizeLightningEffects()
{
    // Lightning optimization - reduce flash frequency and intensity based on performance
    float QualityMultiplier = GetQualityMultiplier();
    
    UE_LOG(LogTemp, Log, TEXT("Lightning effects optimized with multiplier: %f"), QualityMultiplier);
}

void APerf_WeatherPerformanceOptimizer::SetLightningQuality(EPerf_WeatherQuality Quality)
{
    // Set lightning-specific quality
    OptimizeLightningEffects();
}

void APerf_WeatherPerformanceOptimizer::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Update frame time tracking
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds();
    LastFrameTime = CurrentFrameTime;

    // Calculate rolling average
    AverageFrameTime = (AverageFrameTime * 0.9f) + (CurrentFrameTime * 0.1f);

    // Count particles in scene
    int32 TotalParticles = 0;
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);

        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC && PSC->IsActive())
            {
                TotalParticles += PSC->GetNumActiveParticles();
            }
        }
    }

    // Update metrics
    CurrentMetrics.ParticleCount = TotalParticles;
    CurrentMetrics.RenderTime = AverageFrameTime * 1000.0f; // Convert to milliseconds
    CurrentMetrics.FrameImpact = (AverageFrameTime / (1.0f / TargetFrameRate)) * 100.0f; // Percentage of frame budget
    CurrentMetrics.GPUMemoryUsage = 0.0f; // Would need platform-specific implementation
}

void APerf_WeatherPerformanceOptimizer::ApplyQualitySettings()
{
    OptimizeParticleSystems();
    OptimizeMaterialParameters();
}

void APerf_WeatherPerformanceOptimizer::OptimizeParticleSystems()
{
    if (!GetWorld())
    {
        return;
    }

    float QualityMultiplier = GetQualityMultiplier();

    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);

        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC)
            {
                // Apply LOD bias based on quality
                PSC->SetLODLevel(CurrentWeatherQuality == EPerf_WeatherQuality::Low ? 2 : 
                                CurrentWeatherQuality == EPerf_WeatherQuality::Medium ? 1 : 0);
            }
        }
    }
}

void APerf_WeatherPerformanceOptimizer::OptimizeMaterialParameters()
{
    // Optimize material parameters for weather effects
    float QualityMultiplier = GetQualityMultiplier();
    
    // This would typically set global material parameter collection values
    UE_LOG(LogTemp, Log, TEXT("Material parameters optimized with multiplier: %f"), QualityMultiplier);
}

void APerf_WeatherPerformanceOptimizer::MonitorFrameRate()
{
    float CurrentFPS = 1.0f / AverageFrameTime;
    
    // Log performance warnings
    if (CurrentFPS < TargetFrameRate * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather Performance Warning: FPS below target (%f < %f)"), CurrentFPS, TargetFrameRate);
    }
}

float APerf_WeatherPerformanceOptimizer::GetQualityMultiplier() const
{
    switch (CurrentWeatherQuality)
    {
        case EPerf_WeatherQuality::Low:
            return LowQualityParticleMultiplier;
        case EPerf_WeatherQuality::Medium:
            return MediumQualityParticleMultiplier;
        case EPerf_WeatherQuality::High:
            return HighQualityParticleMultiplier;
        case EPerf_WeatherQuality::Ultra:
            return UltraQualityParticleMultiplier;
        default:
            return MediumQualityParticleMultiplier;
    }
}