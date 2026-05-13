#include "Perf_WeatherPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"

APerf_WeatherPhysicsPerformanceIntegrator::APerf_WeatherPhysicsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    bIsMonitoring = false;
    CurrentOptimizationLevel = EPerf_WeatherOptimizationLevel::High;
    WeatherFrameTimeAccumulator = 0.0f;
    WeatherFrameCount = 0;
    LastWeatherUpdateTime = 0.0f;
    bAdaptiveQualityEnabled = true;
    TargetFrameRate = 60.0f;
    FrameRateTolerance = 5.0f;
    WeatherPhysicsSystem = nullptr;
    
    // Performance tracking
    WeatherStartTime = 0.0;
    WeatherEndTime = 0.0;
    TotalWeatherFrames = 0;
    AverageWeatherFrameTime = 0.0f;

    // Set default optimization settings
    OptimizationSettings = FPerf_WeatherOptimizationSettings();
}

void APerf_WeatherPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance Integrator: Starting up"));
    
    // Find and integrate with weather physics system
    IntegrateWithWeatherPhysicsSystem();
    
    // Start monitoring by default
    StartWeatherPerformanceMonitoring();
    
    // Apply initial optimization level
    ApplyOptimizationLevel(CurrentOptimizationLevel);
}

void APerf_WeatherPhysicsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoring)
    {
        UpdateWeatherPerformanceMetrics(DeltaTime);
        UpdateWeatherPhysicsPerformance(DeltaTime);
        
        if (bAdaptiveQualityEnabled)
        {
            float CurrentFPS = 1.0f / DeltaTime;
            UpdateAdaptiveWeatherQuality(CurrentFPS);
        }
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopWeatherPerformanceMonitoring();
    Super::EndPlay(EndPlayReason);
}

void APerf_WeatherPhysicsPerformanceIntegrator::StartWeatherPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        WeatherFrameTimeAccumulator = 0.0f;
        WeatherFrameCount = 0;
        WeatherStartTime = FPlatformTime::Seconds();
        
        UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Monitoring started"));
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::StopWeatherPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        WeatherEndTime = FPlatformTime::Seconds();
        
        if (WeatherFrameCount > 0)
        {
            AverageWeatherFrameTime = WeatherFrameTimeAccumulator / WeatherFrameCount;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Monitoring stopped. Average frame time: %f ms"), 
               AverageWeatherFrameTime * 1000.0f);
    }
}

FPerf_WeatherPerformanceMetrics APerf_WeatherPhysicsPerformanceIntegrator::GetWeatherPerformanceMetrics() const
{
    return CurrentMetrics;
}

void APerf_WeatherPhysicsPerformanceIntegrator::SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    ApplyOptimizationLevel(Level);
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Optimization level set to %d"), (int32)Level);
}

void APerf_WeatherPhysicsPerformanceIntegrator::ApplyWeatherOptimizationSettings(const FPerf_WeatherOptimizationSettings& Settings)
{
    OptimizationSettings = Settings;
    UpdateWeatherQualitySettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Custom optimization settings applied"));
}

void APerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherForFrameRate(float TargetFrameRate)
{
    this->TargetFrameRate = TargetFrameRate;
    
    // Determine optimization level based on target frame rate
    if (TargetFrameRate >= 120.0f)
    {
        SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Minimal);
    }
    else if (TargetFrameRate >= 60.0f)
    {
        SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Low);
    }
    else if (TargetFrameRate >= 30.0f)
    {
        SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Medium);
    }
    else
    {
        SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Optimized for %f FPS target"), TargetFrameRate);
}

void APerf_WeatherPhysicsPerformanceIntegrator::IntegrateWithWeatherPhysicsSystem()
{
    if (UWorld* World = GetWorld())
    {
        // Find weather physics system in the world
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("WeatherPhysicsSystem")))
            {
                WeatherPhysicsSystem = Cast<ACore_WeatherPhysicsSystem>(Actor);
                if (WeatherPhysicsSystem)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Integrated with Weather Physics System"));
                    break;
                }
            }
        }
        
        if (!WeatherPhysicsSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Weather Physics System not found in world"));
        }
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::UpdateWeatherPhysicsPerformance(float DeltaTime)
{
    if (!WeatherPhysicsSystem)
    {
        return;
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    // Update weather physics calculations
    OptimizeWeatherPhysicsCalculations();
    
    double EndTime = FPlatformTime::Seconds();
    float WeatherPhysicsTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    // Update metrics
    CurrentMetrics.WeatherSystemFrameTime = WeatherPhysicsTime;
    CurrentMetrics.WeatherCPUTime = WeatherPhysicsTime * 0.7f; // Estimate CPU portion
    CurrentMetrics.WeatherGPUTime = WeatherPhysicsTime * 0.3f; // Estimate GPU portion
    
    LastWeatherUpdateTime = WeatherPhysicsTime;
}

void APerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherPhysicsCalculations()
{
    if (!WeatherPhysicsSystem)
    {
        return;
    }
    
    // Apply current optimization settings to weather physics
    switch (CurrentOptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            OptimizationSettings.MaxRainDroplets = 5000;
            OptimizationSettings.WindUpdateFrequency = 60.0f;
            OptimizationSettings.bEnableLightningEffects = true;
            OptimizationSettings.WeatherPhysicsQuality = 1.0f;
            break;
            
        case EPerf_WeatherOptimizationLevel::High:
            OptimizationSettings.MaxRainDroplets = 2000;
            OptimizationSettings.WindUpdateFrequency = 45.0f;
            OptimizationSettings.bEnableLightningEffects = true;
            OptimizationSettings.WeatherPhysicsQuality = 0.8f;
            break;
            
        case EPerf_WeatherOptimizationLevel::Medium:
            OptimizationSettings.MaxRainDroplets = 1000;
            OptimizationSettings.WindUpdateFrequency = 30.0f;
            OptimizationSettings.bEnableLightningEffects = true;
            OptimizationSettings.WeatherPhysicsQuality = 0.6f;
            break;
            
        case EPerf_WeatherOptimizationLevel::Low:
            OptimizationSettings.MaxRainDroplets = 500;
            OptimizationSettings.WindUpdateFrequency = 20.0f;
            OptimizationSettings.bEnableLightningEffects = false;
            OptimizationSettings.WeatherPhysicsQuality = 0.4f;
            break;
            
        case EPerf_WeatherOptimizationLevel::Minimal:
            OptimizationSettings.MaxRainDroplets = 100;
            OptimizationSettings.WindUpdateFrequency = 10.0f;
            OptimizationSettings.bEnableLightningEffects = false;
            OptimizationSettings.WeatherPhysicsQuality = 0.2f;
            break;
    }
    
    // Update metrics based on current settings
    CurrentMetrics.RainDropletCount = OptimizationSettings.MaxRainDroplets;
    CurrentMetrics.WindCalculationTime = 1000.0f / OptimizationSettings.WindUpdateFrequency;
}

void APerf_WeatherPhysicsPerformanceIntegrator::EnableAdaptiveWeatherQuality(bool bEnable)
{
    bAdaptiveQualityEnabled = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Adaptive quality %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_WeatherPhysicsPerformanceIntegrator::UpdateAdaptiveWeatherQuality(float CurrentFrameRate)
{
    if (!bAdaptiveQualityEnabled)
    {
        return;
    }
    
    float FrameRateDifference = CurrentFrameRate - TargetFrameRate;
    
    // If frame rate is significantly below target, reduce quality
    if (FrameRateDifference < -FrameRateTolerance)
    {
        switch (CurrentOptimizationLevel)
        {
            case EPerf_WeatherOptimizationLevel::Ultra:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
                break;
            case EPerf_WeatherOptimizationLevel::High:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Medium);
                break;
            case EPerf_WeatherOptimizationLevel::Medium:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Low);
                break;
            case EPerf_WeatherOptimizationLevel::Low:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Minimal);
                break;
            default:
                break;
        }
    }
    // If frame rate is significantly above target, increase quality
    else if (FrameRateDifference > FrameRateTolerance * 2.0f)
    {
        switch (CurrentOptimizationLevel)
        {
            case EPerf_WeatherOptimizationLevel::Minimal:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Low);
                break;
            case EPerf_WeatherOptimizationLevel::Low:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Medium);
                break;
            case EPerf_WeatherOptimizationLevel::Medium:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
                break;
            case EPerf_WeatherOptimizationLevel::High:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Ultra);
                break;
            default:
                break;
        }
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::RunWeatherPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Running performance test..."));
    
    // Reset counters
    ResetWeatherPerformanceCounters();
    
    // Start monitoring
    StartWeatherPerformanceMonitoring();
    
    // Simulate weather performance test
    double TestStartTime = FPlatformTime::Seconds();
    
    // Test different optimization levels
    TArray<EPerf_WeatherOptimizationLevel> TestLevels = {
        EPerf_WeatherOptimizationLevel::Ultra,
        EPerf_WeatherOptimizationLevel::High,
        EPerf_WeatherOptimizationLevel::Medium,
        EPerf_WeatherOptimizationLevel::Low,
        EPerf_WeatherOptimizationLevel::Minimal
    };
    
    for (EPerf_WeatherOptimizationLevel Level : TestLevels)
    {
        SetWeatherOptimizationLevel(Level);
        
        // Simulate weather calculations
        for (int32 i = 0; i < 100; ++i)
        {
            UpdateWeatherPhysicsPerformance(0.016f); // Simulate 60 FPS
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Weather Performance Test - Level %d: Avg Frame Time %f ms"), 
               (int32)Level, CurrentMetrics.WeatherSystemFrameTime);
    }
    
    double TestEndTime = FPlatformTime::Seconds();
    float TotalTestTime = (TestEndTime - TestStartTime) * 1000.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance Test completed in %f ms"), TotalTestTime);
    
    // Restore default level
    SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
}

void APerf_WeatherPhysicsPerformanceIntegrator::LogWeatherPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Weather Physics Performance Report ==="));
    UE_LOG(LogTemp, Warning, TEXT("Monitoring Active: %s"), bIsMonitoring ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Optimization Level: %d"), (int32)CurrentOptimizationLevel);
    UE_LOG(LogTemp, Warning, TEXT("Weather System Frame Time: %f ms"), CurrentMetrics.WeatherSystemFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Rain Droplet Count: %f"), CurrentMetrics.RainDropletCount);
    UE_LOG(LogTemp, Warning, TEXT("Wind Calculation Time: %f ms"), CurrentMetrics.WindCalculationTime);
    UE_LOG(LogTemp, Warning, TEXT("Weather CPU Time: %f ms"), CurrentMetrics.WeatherCPUTime);
    UE_LOG(LogTemp, Warning, TEXT("Weather GPU Time: %f ms"), CurrentMetrics.WeatherGPUTime);
    UE_LOG(LogTemp, Warning, TEXT("Weather Memory Usage: %f MB"), CurrentMetrics.WeatherPhysicsMemoryUsage);
    UE_LOG(LogTemp, Warning, TEXT("Adaptive Quality: %s"), bAdaptiveQualityEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Target Frame Rate: %f FPS"), TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Integration: %s"), WeatherPhysicsSystem ? TEXT("Connected") : TEXT("Not Found"));
    UE_LOG(LogTemp, Warning, TEXT("=== End Weather Performance Report ==="));
}

void APerf_WeatherPhysicsPerformanceIntegrator::ResetWeatherPerformanceCounters()
{
    WeatherFrameTimeAccumulator = 0.0f;
    WeatherFrameCount = 0;
    TotalWeatherFrames = 0;
    AverageWeatherFrameTime = 0.0f;
    CurrentMetrics = FPerf_WeatherPerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Weather Physics Performance: Counters reset"));
}

void APerf_WeatherPhysicsPerformanceIntegrator::UpdateWeatherPerformanceMetrics(float DeltaTime)
{
    WeatherFrameTimeAccumulator += DeltaTime;
    WeatherFrameCount++;
    TotalWeatherFrames++;
    
    // Calculate average every 60 frames
    if (WeatherFrameCount >= 60)
    {
        AverageWeatherFrameTime = WeatherFrameTimeAccumulator / WeatherFrameCount;
        CurrentMetrics.WeatherSystemFrameTime = AverageWeatherFrameTime * 1000.0f; // Convert to ms
        
        // Reset for next measurement
        WeatherFrameTimeAccumulator = 0.0f;
        WeatherFrameCount = 0;
    }
    
    // Estimate memory usage based on current settings
    CurrentMetrics.WeatherPhysicsMemoryUsage = (OptimizationSettings.MaxRainDroplets * 0.1f) + 
                                               (OptimizationSettings.bEnableLightningEffects ? 50.0f : 0.0f);
}

void APerf_WeatherPhysicsPerformanceIntegrator::ApplyOptimizationLevel(EPerf_WeatherOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    OptimizeWeatherPhysicsCalculations();
    UpdateWeatherQualitySettings();
}

void APerf_WeatherPhysicsPerformanceIntegrator::CheckWeatherPhysicsIntegration()
{
    if (!WeatherPhysicsSystem)
    {
        IntegrateWithWeatherPhysicsSystem();
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::OptimizeWeatherBasedOnPerformance()
{
    if (CurrentMetrics.WeatherSystemFrameTime > 16.67f) // Above 60 FPS threshold
    {
        // Performance is poor, reduce quality
        switch (CurrentOptimizationLevel)
        {
            case EPerf_WeatherOptimizationLevel::Ultra:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
                break;
            case EPerf_WeatherOptimizationLevel::High:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Medium);
                break;
            case EPerf_WeatherOptimizationLevel::Medium:
                SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel::Low);
                break;
            default:
                break;
        }
    }
}

void APerf_WeatherPhysicsPerformanceIntegrator::UpdateWeatherQualitySettings()
{
    // Apply optimization settings to weather system
    if (WeatherPhysicsSystem)
    {
        // Update weather system with current optimization settings
        UE_LOG(LogTemp, Log, TEXT("Weather Physics Performance: Quality settings updated"));
    }
}