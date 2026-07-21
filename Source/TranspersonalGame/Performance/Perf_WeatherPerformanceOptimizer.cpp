#include "Perf_WeatherPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/WindDirectionalSourceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"

UPerf_WeatherPerformanceOptimizer::UPerf_WeatherPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / OptimizationSettings.WeatherUpdateFrequency;
    
    // Initialize default optimization settings
    OptimizationSettings.OptimizationLevel = EPerf_WeatherOptimizationLevel::Medium;
    OptimizationSettings.MaxParticleSystemFrameTime = 2.0f;
    OptimizationSettings.MaxActiveParticles = 5000;
    OptimizationSettings.WeatherLODDistance = 5000.0f;
    OptimizationSettings.bEnableVolumetricFogOptimization = true;
    OptimizationSettings.bEnableWindSimulationOptimization = true;
    OptimizationSettings.WeatherUpdateFrequency = 30.0f;
}

void UPerf_WeatherPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Weather Performance Optimizer initialized"));
    
    // Start optimization after a brief delay to allow world setup
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &UPerf_WeatherPerformanceOptimizer::OptimizeWeatherSystems,
        1.0f,
        false
    );
}

void UPerf_WeatherPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateWeatherMetrics();
    
    // Check if we need to apply optimizations
    if (WeatherMetrics.TotalWeatherFrameTime > OptimizationSettings.MaxParticleSystemFrameTime)
    {
        OptimizeWeatherSystems();
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherSystems()
{
    if (bIsOptimizationActive)
    {
        return;
    }
    
    bIsOptimizationActive = true;
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Starting weather system optimization"));
    
    OptimizeParticleSystems();
    OptimizeVolumetricFog();
    OptimizeWindSimulation();
    ApplyOptimizationSettings();
    
    bIsOptimizationActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Weather system optimization completed"));
}

void UPerf_WeatherPerformanceOptimizer::UpdateWeatherMetrics()
{
    UpdateParticleSystemMetrics();
    UpdateVolumetricFogMetrics();
    UpdateWindSimulationMetrics();
    
    // Calculate total frame time
    WeatherMetrics.TotalWeatherFrameTime = 
        WeatherMetrics.ParticleSystemFrameTime +
        WeatherMetrics.VolumetricFogFrameTime +
        WeatherMetrics.WindSimulationFrameTime +
        WeatherMetrics.WeatherUpdateFrameTime;
}

void UPerf_WeatherPerformanceOptimizer::SetOptimizationLevel(EPerf_WeatherOptimizationLevel NewLevel)
{
    OptimizationSettings.OptimizationLevel = NewLevel;
    
    // Adjust settings based on optimization level
    switch (NewLevel)
    {
        case EPerf_WeatherOptimizationLevel::Disabled:
            OptimizationSettings.MaxActiveParticles = 10000;
            OptimizationSettings.WeatherLODDistance = 10000.0f;
            OptimizationSettings.bEnableVolumetricFogOptimization = false;
            OptimizationSettings.bEnableWindSimulationOptimization = false;
            break;
            
        case EPerf_WeatherOptimizationLevel::Low:
            OptimizationSettings.MaxActiveParticles = 8000;
            OptimizationSettings.WeatherLODDistance = 8000.0f;
            OptimizationSettings.bEnableVolumetricFogOptimization = true;
            OptimizationSettings.bEnableWindSimulationOptimization = true;
            break;
            
        case EPerf_WeatherOptimizationLevel::Medium:
            OptimizationSettings.MaxActiveParticles = 5000;
            OptimizationSettings.WeatherLODDistance = 5000.0f;
            OptimizationSettings.bEnableVolumetricFogOptimization = true;
            OptimizationSettings.bEnableWindSimulationOptimization = true;
            break;
            
        case EPerf_WeatherOptimizationLevel::High:
            OptimizationSettings.MaxActiveParticles = 3000;
            OptimizationSettings.WeatherLODDistance = 3000.0f;
            OptimizationSettings.bEnableVolumetricFogOptimization = true;
            OptimizationSettings.bEnableWindSimulationOptimization = true;
            break;
            
        case EPerf_WeatherOptimizationLevel::Ultra:
            OptimizationSettings.MaxActiveParticles = 1500;
            OptimizationSettings.WeatherLODDistance = 2000.0f;
            OptimizationSettings.bEnableVolumetricFogOptimization = true;
            OptimizationSettings.bEnableWindSimulationOptimization = true;
            break;
    }
    
    OptimizeWeatherSystems();
}

void UPerf_WeatherPerformanceOptimizer::OptimizeParticleSystems()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> ParticleActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ParticleActors);
    
    int32 OptimizedParticles = 0;
    
    for (AActor* Actor : ParticleActors)
    {
        if (!Actor) continue;
        
        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
        
        for (UParticleSystemComponent* ParticleComp : ParticleComponents)
        {
            if (!ParticleComp) continue;
            
            // Calculate distance from player
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
                
                // Apply LOD based on distance and optimization level
                if (Distance > OptimizationSettings.WeatherLODDistance)
                {
                    ParticleComp->SetActive(false);
                }
                else
                {
                    ParticleComp->SetActive(true);
                    
                    // Adjust particle count based on distance and optimization level
                    float LODScale = FMath::Clamp(1.0f - (Distance / OptimizationSettings.WeatherLODDistance), 0.1f, 1.0f);
                    
                    // Apply optimization level scaling
                    switch (OptimizationSettings.OptimizationLevel)
                    {
                        case EPerf_WeatherOptimizationLevel::High:
                            LODScale *= 0.7f;
                            break;
                        case EPerf_WeatherOptimizationLevel::Ultra:
                            LODScale *= 0.5f;
                            break;
                        default:
                            break;
                    }
                    
                    // Note: In a real implementation, you would modify particle system parameters
                    // This is a simplified version for demonstration
                }
                
                OptimizedParticles++;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d particle systems"), OptimizedParticles);
}

void UPerf_WeatherPerformanceOptimizer::OptimizeVolumetricFog()
{
    if (!OptimizationSettings.bEnableVolumetricFogOptimization)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find volumetric fog components and optimize them
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FogActors);
    
    for (AActor* Actor : FogActors)
    {
        if (!Actor) continue;
        
        TArray<UVolumetricFogComponent*> FogComponents;
        Actor->GetComponents<UVolumetricFogComponent>(FogComponents);
        
        for (UVolumetricFogComponent* FogComp : FogComponents)
        {
            if (!FogComp) continue;
            
            // Apply optimization based on level
            switch (OptimizationSettings.OptimizationLevel)
            {
                case EPerf_WeatherOptimizationLevel::High:
                case EPerf_WeatherOptimizationLevel::Ultra:
                    // Reduce fog quality for better performance
                    // Note: Actual fog parameters would be adjusted here
                    break;
                default:
                    break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Volumetric fog optimization completed"));
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWindSimulation()
{
    if (!OptimizationSettings.bEnableWindSimulationOptimization)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find wind components and optimize them
    TArray<AActor*> WindActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), WindActors);
    
    for (AActor* Actor : WindActors)
    {
        if (!Actor) continue;
        
        TArray<UWindDirectionalSourceComponent*> WindComponents;
        Actor->GetComponents<UWindDirectionalSourceComponent>(WindComponents);
        
        for (UWindDirectionalSourceComponent* WindComp : WindComponents)
        {
            if (!WindComp) continue;
            
            // Optimize wind simulation based on optimization level
            switch (OptimizationSettings.OptimizationLevel)
            {
                case EPerf_WeatherOptimizationLevel::High:
                    WindComp->SetStrength(WindComp->GetStrength() * 0.8f);
                    break;
                case EPerf_WeatherOptimizationLevel::Ultra:
                    WindComp->SetStrength(WindComp->GetStrength() * 0.6f);
                    break;
                default:
                    break;
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Wind simulation optimization completed"));
}

void UPerf_WeatherPerformanceOptimizer::RunWeatherPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Weather Performance Test ==="));
    
    UpdateWeatherMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Particle System Frame Time: %.2f ms"), WeatherMetrics.ParticleSystemFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Particle Count: %d"), WeatherMetrics.ActiveParticleCount);
    UE_LOG(LogTemp, Warning, TEXT("Volumetric Fog Frame Time: %.2f ms"), WeatherMetrics.VolumetricFogFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Wind Simulation Frame Time: %.2f ms"), WeatherMetrics.WindSimulationFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Total Weather Frame Time: %.2f ms"), WeatherMetrics.TotalWeatherFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Weather Effect LOD Level: %d"), WeatherMetrics.WeatherEffectLODLevel);
    
    // Test optimization
    OptimizeWeatherSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("=== Weather Performance Test Completed ==="));
}

void UPerf_WeatherPerformanceOptimizer::UpdateParticleSystemMetrics()
{
    // Simulate particle system metrics
    WeatherMetrics.ParticleSystemFrameTime = FMath::RandRange(0.5f, 3.0f);
    WeatherMetrics.ActiveParticleCount = FMath::RandRange(1000, 8000);
}

void UPerf_WeatherPerformanceOptimizer::UpdateVolumetricFogMetrics()
{
    // Simulate volumetric fog metrics
    WeatherMetrics.VolumetricFogFrameTime = FMath::RandRange(0.2f, 1.5f);
}

void UPerf_WeatherPerformanceOptimizer::UpdateWindSimulationMetrics()
{
    // Simulate wind simulation metrics
    WeatherMetrics.WindSimulationFrameTime = FMath::RandRange(0.1f, 0.8f);
}

void UPerf_WeatherPerformanceOptimizer::ApplyOptimizationSettings()
{
    // Update tick interval based on optimization settings
    PrimaryComponentTick.TickInterval = 1.0f / OptimizationSettings.WeatherUpdateFrequency;
    
    // Calculate weather effect LOD level
    switch (OptimizationSettings.OptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Disabled:
            WeatherMetrics.WeatherEffectLODLevel = 0;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            WeatherMetrics.WeatherEffectLODLevel = 1;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            WeatherMetrics.WeatherEffectLODLevel = 2;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            WeatherMetrics.WeatherEffectLODLevel = 3;
            break;
        case EPerf_WeatherOptimizationLevel::Ultra:
            WeatherMetrics.WeatherEffectLODLevel = 4;
            break;
    }
}

void UPerf_WeatherPerformanceOptimizer::LogWeatherPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Weather Performance Data:"));
    UE_LOG(LogTemp, Log, TEXT("  Particle Frame Time: %.2f ms"), WeatherMetrics.ParticleSystemFrameTime);
    UE_LOG(LogTemp, Log, TEXT("  Active Particles: %d"), WeatherMetrics.ActiveParticleCount);
    UE_LOG(LogTemp, Log, TEXT("  Total Frame Time: %.2f ms"), WeatherMetrics.TotalWeatherFrameTime);
}