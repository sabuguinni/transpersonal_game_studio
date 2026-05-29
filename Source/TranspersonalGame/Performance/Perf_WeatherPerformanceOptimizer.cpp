#include "Perf_WeatherPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"

UPerf_WeatherPerformanceOptimizer::UPerf_WeatherPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check performance 10 times per second
    
    // Initialize default settings
    CurrentComplexity = EPerf_WeatherComplexity::Medium;
    bDynamicOptimizationEnabled = true;
    PerformanceCheckInterval = 1.0f;
    
    // Performance tracking
    CurrentFrameRate = 60.0f;
    AverageFrameRate = 60.0f;
    ActiveParticleCount = 0;
    WeatherRenderTime = 0.0f;
    
    // Internal state
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    LastPerformanceCheck = 0.0f;
    FrameTimeHistory.Reserve(60); // Store last 60 frame times
}

void UPerf_WeatherPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all weather-related particle systems in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                // Find particle system components
                TArray<UParticleSystemComponent*> ParticleComponents;
                Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
                
                for (UParticleSystemComponent* ParticleComp : ParticleComponents)
                {
                    if (ParticleComp && ParticleComp->GetName().Contains(TEXT("Weather")))
                    {
                        WeatherParticleSystems.Add(ParticleComp);
                    }
                }
                
                // Find audio components
                TArray<UAudioComponent*> AudioComponents;
                Actor->GetComponents<UAudioComponent>(AudioComponents);
                
                for (UAudioComponent* AudioComp : AudioComponents)
                {
                    if (AudioComp && AudioComp->GetName().Contains(TEXT("Weather")))
                    {
                        WeatherAudioComponents.Add(AudioComp);
                    }
                }
            }
        }
    }
    
    // Initialize performance monitoring
    OptimizeWeatherSystems();
    
    UE_LOG(LogTemp, Log, TEXT("Weather Performance Optimizer initialized with %d particle systems and %d audio components"), 
           WeatherParticleSystems.Num(), WeatherAudioComponents.Num());
}

void UPerf_WeatherPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Calculate current frame rate
    CalculateFrameRate(DeltaTime);
    
    // Check if it's time for performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        MonitorWeatherPerformance();
        
        if (bDynamicOptimizationEnabled)
        {
            CheckPerformanceThresholds();
        }
        
        LastPerformanceCheck = 0.0f;
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherSystems()
{
    OptimizeRainSystem();
    OptimizeSnowSystem();
    OptimizeFogSystem();
    OptimizeWindSystem();
    OptimizeWeatherAudio();
    OptimizeWeatherLighting();
    AdjustLODSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Weather systems optimized for complexity level: %d"), (int32)CurrentComplexity);
}

void UPerf_WeatherPerformanceOptimizer::SetWeatherComplexity(EPerf_WeatherComplexity Complexity)
{
    CurrentComplexity = Complexity;
    
    // Adjust performance settings based on complexity
    switch (Complexity)
    {
        case EPerf_WeatherComplexity::Minimal:
            PerformanceSettings.MaxRainParticles = 100;
            PerformanceSettings.MaxSnowParticles = 50;
            PerformanceSettings.MaxFogParticles = 25;
            PerformanceSettings.bEnableDynamicWeatherLighting = false;
            break;
            
        case EPerf_WeatherComplexity::Low:
            PerformanceSettings.MaxRainParticles = 300;
            PerformanceSettings.MaxSnowParticles = 150;
            PerformanceSettings.MaxFogParticles = 75;
            PerformanceSettings.bEnableDynamicWeatherLighting = false;
            break;
            
        case EPerf_WeatherComplexity::Medium:
            PerformanceSettings.MaxRainParticles = 600;
            PerformanceSettings.MaxSnowParticles = 300;
            PerformanceSettings.MaxFogParticles = 150;
            PerformanceSettings.bEnableDynamicWeatherLighting = true;
            break;
            
        case EPerf_WeatherComplexity::High:
            PerformanceSettings.MaxRainParticles = 1000;
            PerformanceSettings.MaxSnowParticles = 500;
            PerformanceSettings.MaxFogParticles = 250;
            PerformanceSettings.bEnableDynamicWeatherLighting = true;
            break;
            
        case EPerf_WeatherComplexity::Ultra:
            PerformanceSettings.MaxRainParticles = 2000;
            PerformanceSettings.MaxSnowParticles = 1000;
            PerformanceSettings.MaxFogParticles = 500;
            PerformanceSettings.bEnableDynamicWeatherLighting = true;
            break;
    }
    
    OptimizeWeatherSystems();
}

void UPerf_WeatherPerformanceOptimizer::AdjustParticleCount(int32 NewParticleCount)
{
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->IsValidLowLevel())
        {
            // Adjust particle count based on system type
            if (ParticleComp->GetName().Contains(TEXT("Rain")))
            {
                PerformanceSettings.MaxRainParticles = FMath::Min(NewParticleCount, 2000);
            }
            else if (ParticleComp->GetName().Contains(TEXT("Snow")))
            {
                PerformanceSettings.MaxSnowParticles = FMath::Min(NewParticleCount / 2, 1000);
            }
            else if (ParticleComp->GetName().Contains(TEXT("Fog")))
            {
                PerformanceSettings.MaxFogParticles = FMath::Min(NewParticleCount / 4, 500);
            }
        }
    }
    
    UpdateParticleSettings();
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherAudio()
{
    for (UAudioComponent* AudioComp : WeatherAudioComponents)
    {
        if (AudioComp && AudioComp->IsValidLowLevel())
        {
            // Adjust audio settings based on performance requirements
            AudioComp->SetVolumeMultiplier(PerformanceSettings.WeatherAudioVolume);
            
            // Adjust attenuation distance
            if (AudioComp->AttenuationSettings)
            {
                AudioComp->AttenuationSettings->Attenuation.FalloffDistance = PerformanceSettings.WeatherAudioDistance;
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherLighting()
{
    if (!PerformanceSettings.bEnableDynamicWeatherLighting)
    {
        // Disable dynamic lighting for weather effects
        UWorld* World = GetWorld();
        if (World)
        {
            // Find and disable dynamic weather lights
            for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                AActor* Actor = *ActorIterator;
                if (Actor && Actor->GetName().Contains(TEXT("WeatherLight")))
                {
                    Actor->SetActorHiddenInGame(true);
                }
            }
        }
    }
}

float UPerf_WeatherPerformanceOptimizer::GetCurrentFrameRate() const
{
    return CurrentFrameRate;
}

bool UPerf_WeatherPerformanceOptimizer::IsPerformanceAcceptable() const
{
    return CurrentFrameRate >= PerformanceSettings.MinFrameRate;
}

void UPerf_WeatherPerformanceOptimizer::MonitorWeatherPerformance()
{
    // Count active particles
    ActiveParticleCount = 0;
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->IsValidLowLevel() && ParticleComp->IsActive())
        {
            ActiveParticleCount += ParticleComp->GetNumActiveParticles();
        }
    }
    
    // Log performance metrics periodically
    static float LastLogTime = 0.0f;
    LastLogTime += PerformanceCheckInterval;
    if (LastLogTime >= 5.0f) // Log every 5 seconds
    {
        LogWeatherPerformanceMetrics();
        LastLogTime = 0.0f;
    }
}

void UPerf_WeatherPerformanceOptimizer::LogWeatherPerformanceMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("Weather Performance - FPS: %.1f, Particles: %d, Complexity: %d"), 
           CurrentFrameRate, ActiveParticleCount, (int32)CurrentComplexity);
}

void UPerf_WeatherPerformanceOptimizer::EnableDynamicOptimization(bool bEnabled)
{
    bDynamicOptimizationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Dynamic weather optimization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_WeatherPerformanceOptimizer::UpdatePerformanceSettings()
{
    UpdateParticleSettings();
    UpdateAudioSettings();
    UpdateLightingSettings();
}

void UPerf_WeatherPerformanceOptimizer::OptimizeRainSystem()
{
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->GetName().Contains(TEXT("Rain")))
        {
            // Adjust rain particle count based on performance settings
            ParticleComp->SetIntParameter(TEXT("ParticleCount"), PerformanceSettings.MaxRainParticles);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeSnowSystem()
{
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->GetName().Contains(TEXT("Snow")))
        {
            // Adjust snow particle count based on performance settings
            ParticleComp->SetIntParameter(TEXT("ParticleCount"), PerformanceSettings.MaxSnowParticles);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeFogSystem()
{
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->GetName().Contains(TEXT("Fog")))
        {
            // Adjust fog particle count based on performance settings
            ParticleComp->SetIntParameter(TEXT("ParticleCount"), PerformanceSettings.MaxFogParticles);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWindSystem()
{
    // Optimize wind effects based on complexity level
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("Wind")))
            {
                // Adjust wind effect intensity based on performance
                if (CurrentComplexity <= EPerf_WeatherComplexity::Low)
                {
                    Actor->SetActorHiddenInGame(true);
                }
                else
                {
                    Actor->SetActorHiddenInGame(false);
                }
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::AdjustLODSettings()
{
    // Adjust LOD settings for weather effects based on performance
    for (UParticleSystemComponent* ParticleComp : WeatherParticleSystems)
    {
        if (ParticleComp && ParticleComp->IsValidLowLevel())
        {
            // Set LOD distance based on complexity
            float LODDistance = 1000.0f;
            switch (CurrentComplexity)
            {
                case EPerf_WeatherComplexity::Minimal:
                    LODDistance = 500.0f;
                    break;
                case EPerf_WeatherComplexity::Low:
                    LODDistance = 750.0f;
                    break;
                case EPerf_WeatherComplexity::Medium:
                    LODDistance = 1000.0f;
                    break;
                case EPerf_WeatherComplexity::High:
                    LODDistance = 1500.0f;
                    break;
                case EPerf_WeatherComplexity::Ultra:
                    LODDistance = 2000.0f;
                    break;
            }
            
            ParticleComp->SetFloatParameter(TEXT("LODDistance"), LODDistance);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::UpdateParticleSettings()
{
    OptimizeRainSystem();
    OptimizeSnowSystem();
    OptimizeFogSystem();
}

void UPerf_WeatherPerformanceOptimizer::UpdateAudioSettings()
{
    OptimizeWeatherAudio();
}

void UPerf_WeatherPerformanceOptimizer::UpdateLightingSettings()
{
    OptimizeWeatherLighting();
}

void UPerf_WeatherPerformanceOptimizer::CalculateFrameRate(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        CurrentFrameRate = 1.0f / DeltaTime;
        
        // Add to frame time history
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > 60)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average frame rate
        if (FrameTimeHistory.Num() > 0)
        {
            float AverageFrameTime = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                AverageFrameTime += FrameTime;
            }
            AverageFrameTime /= FrameTimeHistory.Num();
            AverageFrameRate = 1.0f / AverageFrameTime;
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::CheckPerformanceThresholds()
{
    if (CurrentFrameRate < PerformanceSettings.MinFrameRate)
    {
        // Performance is below threshold, reduce complexity
        if (CurrentComplexity > EPerf_WeatherComplexity::Minimal)
        {
            EPerf_WeatherComplexity NewComplexity = static_cast<EPerf_WeatherComplexity>(
                static_cast<int32>(CurrentComplexity) - 1);
            SetWeatherComplexity(NewComplexity);
            
            UE_LOG(LogTemp, Warning, TEXT("Performance below threshold (%.1f FPS), reducing weather complexity to %d"), 
                   CurrentFrameRate, (int32)NewComplexity);
        }
    }
    else if (CurrentFrameRate > PerformanceSettings.TargetFrameRate + 10.0f)
    {
        // Performance is well above target, can increase complexity
        if (CurrentComplexity < EPerf_WeatherComplexity::Ultra)
        {
            EPerf_WeatherComplexity NewComplexity = static_cast<EPerf_WeatherComplexity>(
                static_cast<int32>(CurrentComplexity) + 1);
            SetWeatherComplexity(NewComplexity);
            
            UE_LOG(LogTemp, Log, TEXT("Performance above target (%.1f FPS), increasing weather complexity to %d"), 
                   CurrentFrameRate, (int32)NewComplexity);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::ApplyPerformanceAdjustments()
{
    UpdatePerformanceSettings();
}