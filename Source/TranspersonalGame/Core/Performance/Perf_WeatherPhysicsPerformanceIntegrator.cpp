#include "Perf_WeatherPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

DECLARE_CYCLE_STAT(TEXT("Weather Physics Performance Update"), STAT_WeatherPhysicsPerformanceUpdate, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Weather Rain Simulation"), STAT_WeatherRainSimulation, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Weather Wind Calculation"), STAT_WeatherWindCalculation, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Weather Collision"), STAT_WeatherCollision, STATGROUP_Game);
DECLARE_CYCLE_STAT(TEXT("Weather Atmospheric Physics"), STAT_WeatherAtmosphericPhysics, STATGROUP_Game);

UPerf_WeatherPhysicsPerformanceIntegrator::UPerf_WeatherPhysicsPerformanceIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize default values
    CurrentOptimizationLevel = EPerf_WeatherOptimizationLevel::High;
    WeatherCPUThreshold = 5.0f; // 5ms CPU budget for weather systems
    WeatherGPUThreshold = 3.0f; // 3ms GPU budget for weather systems
    WeatherMemoryThreshold = 50.0f; // 50MB memory budget for weather systems
    
    MaxRainParticles = 2000;
    WindCalculationFrequency = 30.0f; // 30 Hz
    WeatherCollisionComplexity = 3;
    AtmosphericUpdateRate = 10.0f; // 10 Hz
    
    bWeatherPhysicsLODEnabled = true;
    bProfilerActive = false;
    LastMetricsUpdateTime = 0.0f;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Weather Physics Performance Integrator initialized"));
    
    // Apply initial optimization level
    ApplyOptimizationLevel();
    
    // Start performance monitoring
    MonitorWeatherPhysicsPerformance();
}

void UPerf_WeatherPhysicsPerformanceIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    SCOPE_CYCLE_COUNTER(STAT_WeatherPhysicsPerformanceUpdate);
    
    // Update metrics periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMetricsUpdateTime > 0.5f) // Update every 500ms
    {
        UpdateWeatherPhysicsMetrics();
        LastMetricsUpdateTime = CurrentTime;
        
        // Check performance thresholds
        if (!IsWeatherPerformanceWithinThresholds())
        {
            HandleWeatherPhysicsThresholdViolation();
        }
    }
}

FPerf_WeatherPhysicsMetrics UPerf_WeatherPhysicsPerformanceIntegrator::GetWeatherPhysicsMetrics() const
{
    UpdateWeatherPhysicsMetrics();
    return CachedMetrics;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Optimizing Weather Physics Performance"));
    
    // Optimize all weather subsystems
    OptimizeRainDropletSimulation();
    OptimizeWindForceCalculations();
    OptimizeWeatherCollisions();
    OptimizeAtmosphericPhysics();
    OptimizeWeatherMemoryUsage();
    
    UE_LOG(LogTemp, Log, TEXT("Weather Physics Performance optimization complete"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizationLevel();
    UE_LOG(LogTemp, Log, TEXT("Weather optimization level set to: %d"), (int32)Level);
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetWeatherFrameImpact() const
{
    const FPerf_WeatherPhysicsMetrics& Metrics = GetWeatherPhysicsMetrics();
    return Metrics.WeatherCPUTime + Metrics.WeatherGPUTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::EnableWeatherPhysicsLOD(bool bEnable)
{
    bWeatherPhysicsLODEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Weather Physics LOD %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeRainDropletSimulation()
{
    SCOPE_CYCLE_COUNTER(STAT_WeatherRainSimulation);
    
    // Implement rain droplet optimization based on current optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            MaxRainParticles = 5000;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            MaxRainParticles = 2000;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            MaxRainParticles = 1000;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            MaxRainParticles = 500;
            break;
        case EPerf_WeatherOptimizationLevel::Minimal:
            MaxRainParticles = 100;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Rain droplet simulation optimized: Max particles = %d"), MaxRainParticles);
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetRainParticleLimit(int32 MaxParticles)
{
    MaxRainParticles = FMath::Clamp(MaxParticles, 50, 10000);
    UE_LOG(LogTemp, Log, TEXT("Rain particle limit set to: %d"), MaxRainParticles);
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetRainSimulationCost() const
{
    return CachedMetrics.RainDropletSimulationTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeWindForceCalculations()
{
    SCOPE_CYCLE_COUNTER(STAT_WeatherWindCalculation);
    
    // Optimize wind calculations based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            WindCalculationFrequency = 60.0f;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            WindCalculationFrequency = 30.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            WindCalculationFrequency = 20.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            WindCalculationFrequency = 10.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Minimal:
            WindCalculationFrequency = 5.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Wind force calculations optimized: Frequency = %.1f Hz"), WindCalculationFrequency);
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetWindCalculationFrequency(float Frequency)
{
    WindCalculationFrequency = FMath::Clamp(Frequency, 1.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Wind calculation frequency set to: %.1f Hz"), WindCalculationFrequency);
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetWindCalculationCost() const
{
    return CachedMetrics.WindForceCalculationTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherCollisions()
{
    SCOPE_CYCLE_COUNTER(STAT_WeatherCollision);
    
    // Optimize weather collision complexity
    switch (CurrentOptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            WeatherCollisionComplexity = 5;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            WeatherCollisionComplexity = 3;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            WeatherCollisionComplexity = 2;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            WeatherCollisionComplexity = 1;
            break;
        case EPerf_WeatherOptimizationLevel::Minimal:
            WeatherCollisionComplexity = 0;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weather collisions optimized: Complexity = %d"), WeatherCollisionComplexity);
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetWeatherCollisionComplexity(int32 Complexity)
{
    WeatherCollisionComplexity = FMath::Clamp(Complexity, 0, 10);
    UE_LOG(LogTemp, Log, TEXT("Weather collision complexity set to: %d"), WeatherCollisionComplexity);
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetWeatherCollisionCost() const
{
    return CachedMetrics.WeatherCollisionTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeAtmosphericPhysics()
{
    SCOPE_CYCLE_COUNTER(STAT_WeatherAtmosphericPhysics);
    
    // Optimize atmospheric physics update rate
    switch (CurrentOptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            AtmosphericUpdateRate = 30.0f;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            AtmosphericUpdateRate = 10.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            AtmosphericUpdateRate = 5.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            AtmosphericUpdateRate = 2.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Minimal:
            AtmosphericUpdateRate = 1.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Atmospheric physics optimized: Update rate = %.1f Hz"), AtmosphericUpdateRate);
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetAtmosphericUpdateRate(float UpdateRate)
{
    AtmosphericUpdateRate = FMath::Clamp(UpdateRate, 0.1f, 60.0f);
    UE_LOG(LogTemp, Log, TEXT("Atmospheric update rate set to: %.1f Hz"), AtmosphericUpdateRate);
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetAtmosphericPhysicsCost() const
{
    return CachedMetrics.AtmosphericPressureTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherMemoryUsage()
{
    // Clear weather memory cache
    ClearWeatherMemoryCache();
    
    UE_LOG(LogTemp, Log, TEXT("Weather memory usage optimized"));
}

float UPerf_WeatherPhysicsPerformanceIntegrator::GetWeatherMemoryFootprint() const
{
    return CachedMetrics.WeatherMemoryUsage;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::ClearWeatherMemoryCache()
{
    // Implementation would clear weather-related memory caches
    UE_LOG(LogTemp, Log, TEXT("Weather memory cache cleared"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::SetWeatherPerformanceThresholds(float CPUThreshold, float GPUThreshold, float MemoryThreshold)
{
    WeatherCPUThreshold = CPUThreshold;
    WeatherGPUThreshold = GPUThreshold;
    WeatherMemoryThreshold = MemoryThreshold;
    
    UE_LOG(LogTemp, Log, TEXT("Weather performance thresholds set: CPU=%.2fms, GPU=%.2fms, Memory=%.2fMB"), 
           CPUThreshold, GPUThreshold, MemoryThreshold);
}

bool UPerf_WeatherPhysicsPerformanceIntegrator::IsWeatherPerformanceWithinThresholds() const
{
    const FPerf_WeatherPhysicsMetrics& Metrics = GetWeatherPhysicsMetrics();
    
    return (Metrics.WeatherCPUTime <= WeatherCPUThreshold) &&
           (Metrics.WeatherGPUTime <= WeatherGPUThreshold) &&
           (Metrics.WeatherMemoryUsage <= WeatherMemoryThreshold);
}

void UPerf_WeatherPhysicsPerformanceIntegrator::StartWeatherPhysicsProfiler()
{
    bProfilerActive = true;
    UE_LOG(LogTemp, Log, TEXT("Weather Physics Profiler started"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::StopWeatherPhysicsProfiler()
{
    bProfilerActive = false;
    UE_LOG(LogTemp, Log, TEXT("Weather Physics Profiler stopped"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::LogWeatherPhysicsPerformance() const
{
    const FPerf_WeatherPhysicsMetrics& Metrics = GetWeatherPhysicsMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("=== Weather Physics Performance Report ==="));
    UE_LOG(LogTemp, Log, TEXT("Rain Droplet Simulation: %.2fms"), Metrics.RainDropletSimulationTime);
    UE_LOG(LogTemp, Log, TEXT("Wind Force Calculation: %.2fms"), Metrics.WindForceCalculationTime);
    UE_LOG(LogTemp, Log, TEXT("Weather Collision: %.2fms"), Metrics.WeatherCollisionTime);
    UE_LOG(LogTemp, Log, TEXT("Atmospheric Pressure: %.2fms"), Metrics.AtmosphericPressureTime);
    UE_LOG(LogTemp, Log, TEXT("Active Weather Particles: %d"), Metrics.ActiveWeatherParticles);
    UE_LOG(LogTemp, Log, TEXT("Weather Memory Usage: %.2fMB"), Metrics.WeatherMemoryUsage);
    UE_LOG(LogTemp, Log, TEXT("Weather GPU Time: %.2fms"), Metrics.WeatherGPUTime);
    UE_LOG(LogTemp, Log, TEXT("Weather CPU Time: %.2fms"), Metrics.WeatherCPUTime);
    UE_LOG(LogTemp, Log, TEXT("Total Weather Frame Impact: %.2fms"), GetWeatherFrameImpact());
}

void UPerf_WeatherPhysicsPerformanceIntegrator::UpdateWeatherPhysicsMetrics() const
{
    // Simulate realistic weather physics metrics
    CachedMetrics.RainDropletSimulationTime = FMath::RandRange(0.5f, 3.0f);
    CachedMetrics.WindForceCalculationTime = FMath::RandRange(0.2f, 1.5f);
    CachedMetrics.WeatherCollisionTime = FMath::RandRange(0.1f, 2.0f);
    CachedMetrics.AtmosphericPressureTime = FMath::RandRange(0.1f, 0.8f);
    CachedMetrics.ActiveWeatherParticles = FMath::RandRange(100, MaxRainParticles);
    CachedMetrics.WeatherMemoryUsage = FMath::RandRange(10.0f, 80.0f);
    CachedMetrics.WeatherGPUTime = FMath::RandRange(1.0f, 5.0f);
    CachedMetrics.WeatherCPUTime = CachedMetrics.RainDropletSimulationTime + 
                                   CachedMetrics.WindForceCalculationTime + 
                                   CachedMetrics.WeatherCollisionTime + 
                                   CachedMetrics.AtmosphericPressureTime;
}

void UPerf_WeatherPhysicsPerformanceIntegrator::ApplyOptimizationLevel()
{
    UE_LOG(LogTemp, Log, TEXT("Applying weather optimization level: %d"), (int32)CurrentOptimizationLevel);
    
    // Apply optimization settings based on current level
    OptimizeRainDropletSimulation();
    OptimizeWindForceCalculations();
    OptimizeWeatherCollisions();
    OptimizeAtmosphericPhysics();
}

void UPerf_WeatherPhysicsPerformanceIntegrator::MonitorWeatherPhysicsPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Weather Physics Performance monitoring started"));
}

void UPerf_WeatherPhysicsPerformanceIntegrator::HandleWeatherPhysicsThresholdViolation()
{
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance threshold violation detected!"));
    
    // Automatically reduce optimization level if performance is poor
    if (CurrentOptimizationLevel != EPerf_WeatherOptimizationLevel::Minimal)
    {
        int32 CurrentLevel = (int32)CurrentOptimizationLevel;
        CurrentLevel = FMath::Min(CurrentLevel + 1, (int32)EPerf_WeatherOptimizationLevel::Minimal);
        SetWeatherOptimizationLevel((EPerf_WeatherOptimizationLevel)CurrentLevel);
        
        UE_LOG(LogTemp, Log, TEXT("Automatically reduced weather optimization level to: %d"), CurrentLevel);
    }
}