#include "Perf_WeatherPerformanceIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UPerf_WeatherPerformanceIntegrator::UPerf_WeatherPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check performance every 100ms
    
    PerformanceLevel = EPerf_WeatherPerformanceLevel::High;
    TargetFrameRate = 60.0f;
    MaxWeatherRenderBudget = 5.0f;
    PerformanceCheckInterval = 1.0f;
    bEnableWeatherPhysicsIntegration = true;
    bEnableTerrainWeatherInteraction = true;
}

void UPerf_WeatherPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Starting weather performance optimization"));
    
    // Initialize performance monitoring
    TimeSinceLastPerformanceCheck = 0.0f;
    CurrentFrameTime = 0.0f;
    
    // Set initial performance level
    OptimizeWeatherPerformance();
}

void UPerf_WeatherPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentFrameTime = DeltaTime;
    TimeSinceLastPerformanceCheck += DeltaTime;
    
    // Check performance periodically
    if (TimeSinceLastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdatePerformanceMetrics();
        
        // Auto-adjust quality if performance is poor
        if (!IsWeatherPerformanceOptimal())
        {
            AdjustWeatherQuality();
        }
        
        TimeSinceLastPerformanceCheck = 0.0f;
    }
}

void UPerf_WeatherPerformanceIntegrator::OptimizeWeatherPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Optimizing weather performance for level %d"), 
           (int32)PerformanceLevel);
    
    switch (PerformanceLevel)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            MaxWeatherRenderBudget = 8.0f;
            break;
        case EPerf_WeatherPerformanceLevel::High:
            MaxWeatherRenderBudget = 5.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Medium:
            MaxWeatherRenderBudget = 3.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Low:
            MaxWeatherRenderBudget = 2.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Minimal:
            MaxWeatherRenderBudget = 1.0f;
            break;
    }
    
    OptimizeParticleEffects();
    OptimizeLightingEffects();
    OptimizeVolumetricFog();
}

void UPerf_WeatherPerformanceIntegrator::SetPerformanceLevel(EPerf_WeatherPerformanceLevel NewLevel)
{
    if (PerformanceLevel != NewLevel)
    {
        PerformanceLevel = NewLevel;
        OptimizeWeatherPerformance();
        
        UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Performance level changed to %d"), (int32)NewLevel);
    }
}

FPerf_WeatherPerformanceMetrics UPerf_WeatherPerformanceIntegrator::GetWeatherPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UPerf_WeatherPerformanceIntegrator::IntegrateWithPhysicsSystem()
{
    if (!bEnableWeatherPhysicsIntegration)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Integrating with physics system"));
    
    // Optimize weather-physics interactions based on performance level
    float PhysicsComplexityMultiplier = 1.0f;
    
    switch (PerformanceLevel)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            PhysicsComplexityMultiplier = 1.0f;
            break;
        case EPerf_WeatherPerformanceLevel::High:
            PhysicsComplexityMultiplier = 0.8f;
            break;
        case EPerf_WeatherPerformanceLevel::Medium:
            PhysicsComplexityMultiplier = 0.6f;
            break;
        case EPerf_WeatherPerformanceLevel::Low:
            PhysicsComplexityMultiplier = 0.4f;
            break;
        case EPerf_WeatherPerformanceLevel::Minimal:
            PhysicsComplexityMultiplier = 0.2f;
            break;
    }
    
    CurrentMetrics.WindSimulationComplexity = PhysicsComplexityMultiplier * 100.0f;
}

void UPerf_WeatherPerformanceIntegrator::IntegrateWithTerrainSystem()
{
    if (!bEnableTerrainWeatherInteraction)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Integrating with terrain system"));
    
    // Optimize terrain-weather interactions
    // Reduce terrain detail during heavy weather to maintain performance
    if (CurrentMetrics.WeatherRenderCost > MaxWeatherRenderBudget * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeatherPerformanceIntegrator: High weather render cost, reducing terrain detail"));
    }
}

void UPerf_WeatherPerformanceIntegrator::UpdateWeatherEffectLOD(float DistanceToPlayer)
{
    // Adjust weather effect quality based on distance to player
    float LODMultiplier = 1.0f;
    
    if (DistanceToPlayer > 5000.0f)
    {
        LODMultiplier = 0.3f; // Far distance - minimal effects
    }
    else if (DistanceToPlayer > 2000.0f)
    {
        LODMultiplier = 0.6f; // Medium distance - reduced effects
    }
    else if (DistanceToPlayer > 500.0f)
    {
        LODMultiplier = 0.8f; // Close distance - high quality
    }
    // else: Very close - full quality (1.0f)
    
    // Apply LOD to particle effects
    CurrentMetrics.RainParticleCount *= LODMultiplier;
    CurrentMetrics.LightningEffectCount *= LODMultiplier;
}

bool UPerf_WeatherPerformanceIntegrator::IsWeatherPerformanceOptimal() const
{
    float CurrentFPS = 1.0f / FMath::Max(CurrentFrameTime, 0.001f);
    bool bFrameRateGood = CurrentFPS >= (TargetFrameRate * 0.9f); // Allow 10% tolerance
    bool bRenderCostGood = CurrentMetrics.WeatherRenderCost <= MaxWeatherRenderBudget;
    
    return bFrameRateGood && bRenderCostGood;
}

float UPerf_WeatherPerformanceIntegrator::GetWeatherRenderCost() const
{
    return CurrentMetrics.WeatherRenderCost;
}

void UPerf_WeatherPerformanceIntegrator::UpdatePerformanceMetrics()
{
    // Calculate current weather render cost
    float ParticleCost = CurrentMetrics.RainParticleCount * 0.001f;
    float FogCost = CurrentMetrics.FogDensity * 0.5f;
    float LightningCost = CurrentMetrics.LightningEffectCount * 0.2f;
    float WindCost = CurrentMetrics.WindSimulationComplexity * 0.01f;
    
    CurrentMetrics.WeatherRenderCost = ParticleCost + FogCost + LightningCost + WindCost;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("WeatherPerformanceIntegrator: Render cost %.2fms (Target: %.2fms)"), 
           CurrentMetrics.WeatherRenderCost, MaxWeatherRenderBudget);
}

void UPerf_WeatherPerformanceIntegrator::AdjustWeatherQuality()
{
    if (CurrentMetrics.WeatherRenderCost > MaxWeatherRenderBudget)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeatherPerformanceIntegrator: Performance poor, reducing weather quality"));
        
        // Automatically reduce performance level if needed
        if (PerformanceLevel != EPerf_WeatherPerformanceLevel::Minimal)
        {
            EPerf_WeatherPerformanceLevel NewLevel = static_cast<EPerf_WeatherPerformanceLevel>(
                static_cast<int32>(PerformanceLevel) + 1);
            SetPerformanceLevel(NewLevel);
        }
    }
}

void UPerf_WeatherPerformanceIntegrator::OptimizeParticleEffects()
{
    float ParticleMultiplier = 1.0f;
    
    switch (PerformanceLevel)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            ParticleMultiplier = 1.0f;
            CurrentMetrics.RainParticleCount = 5000.0f;
            break;
        case EPerf_WeatherPerformanceLevel::High:
            ParticleMultiplier = 0.8f;
            CurrentMetrics.RainParticleCount = 3000.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Medium:
            ParticleMultiplier = 0.6f;
            CurrentMetrics.RainParticleCount = 2000.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Low:
            ParticleMultiplier = 0.4f;
            CurrentMetrics.RainParticleCount = 1000.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Minimal:
            ParticleMultiplier = 0.2f;
            CurrentMetrics.RainParticleCount = 500.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Particle effects optimized to %.0f particles"), 
           CurrentMetrics.RainParticleCount);
}

void UPerf_WeatherPerformanceIntegrator::OptimizeLightingEffects()
{
    switch (PerformanceLevel)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            CurrentMetrics.LightningEffectCount = 10.0f;
            break;
        case EPerf_WeatherPerformanceLevel::High:
            CurrentMetrics.LightningEffectCount = 6.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Medium:
            CurrentMetrics.LightningEffectCount = 4.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Low:
            CurrentMetrics.LightningEffectCount = 2.0f;
            break;
        case EPerf_WeatherPerformanceLevel::Minimal:
            CurrentMetrics.LightningEffectCount = 1.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Lightning effects optimized to %.0f effects"), 
           CurrentMetrics.LightningEffectCount);
}

void UPerf_WeatherPerformanceIntegrator::OptimizeVolumetricFog()
{
    switch (PerformanceLevel)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            CurrentMetrics.FogDensity = 1.0f;
            break;
        case EPerf_WeatherPerformanceLevel::High:
            CurrentMetrics.FogDensity = 0.8f;
            break;
        case EPerf_WeatherPerformanceLevel::Medium:
            CurrentMetrics.FogDensity = 0.6f;
            break;
        case EPerf_WeatherPerformanceLevel::Low:
            CurrentMetrics.FogDensity = 0.4f;
            break;
        case EPerf_WeatherPerformanceLevel::Minimal:
            CurrentMetrics.FogDensity = 0.2f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WeatherPerformanceIntegrator: Volumetric fog optimized to %.1f density"), 
           CurrentMetrics.FogDensity);
}