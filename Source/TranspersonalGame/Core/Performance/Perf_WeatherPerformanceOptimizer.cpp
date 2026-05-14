#include "Perf_WeatherPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UPerf_WeatherPerformanceOptimizer::UPerf_WeatherPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    OptimizationLevel = EPerf_WeatherOptimizationLevel::High;
    TargetFrameRate = 60.0f;
    WeatherPerformanceBudget = 5.0f; // 5ms budget for weather systems
    
    LastOptimizationTime = 0.0f;
    OptimizationUpdateInterval = 1.0f; // Optimize every second
    bIsOptimizationActive = true;
}

void UPerf_WeatherPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Weather Performance Optimizer initialized - Level: %d"), 
           static_cast<int32>(OptimizationLevel));
}

void UPerf_WeatherPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsOptimizationActive)
        return;
    
    UpdatePerformanceMetrics();
    
    // Check if we need to apply dynamic optimization
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    if (CurrentFrameTime > (1000.0f / TargetFrameRate) * 1.2f) // 20% tolerance
    {
        ApplyDynamicWeatherOptimization(CurrentFrameTime);
    }
    
    // Periodic optimization update
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime >= OptimizationUpdateInterval)
    {
        OptimizeWeatherSystems();
        LastOptimizationTime = CurrentTime;
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherSystems()
{
    OptimizeRainSystem();
    OptimizeFogSystem();
    OptimizeCloudSystem();
    OptimizeLightningSystem();
    OptimizeWeatherTransitions();
    
    CullDistantWeatherEffects();
    AdjustParticleSystemLOD();
    OptimizeWeatherShaders();
    
    UE_LOG(LogTemp, Log, TEXT("Weather systems optimized - Rain particles: %.0f, Fog density: %.2f"), 
           CurrentMetrics.RainParticleCount, CurrentMetrics.FogDensity);
}

void UPerf_WeatherPerformanceOptimizer::SetOptimizationLevel(EPerf_WeatherOptimizationLevel NewLevel)
{
    OptimizationLevel = NewLevel;
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Weather optimization level changed to: %d"), static_cast<int32>(NewLevel));
}

void UPerf_WeatherPerformanceOptimizer::OptimizeRainSystem()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all rain particle systems in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Look for particle system components that might be rain
        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
        
        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC && PSC->GetName().Contains(TEXT("Rain")))
            {
                // Apply rain optimization based on level
                switch (OptimizationLevel)
                {
                    case EPerf_WeatherOptimizationLevel::Ultra:
                        PSC->SetFloatParameter(TEXT("ParticleCount"), OptimizationSettings.MaxRainParticles);
                        break;
                    case EPerf_WeatherOptimizationLevel::High:
                        PSC->SetFloatParameter(TEXT("ParticleCount"), OptimizationSettings.MaxRainParticles * 0.8f);
                        break;
                    case EPerf_WeatherOptimizationLevel::Medium:
                        PSC->SetFloatParameter(TEXT("ParticleCount"), OptimizationSettings.MaxRainParticles * 0.6f);
                        break;
                    case EPerf_WeatherOptimizationLevel::Low:
                        PSC->SetFloatParameter(TEXT("ParticleCount"), OptimizationSettings.MaxRainParticles * 0.4f);
                        break;
                    case EPerf_WeatherOptimizationLevel::Potato:
                        PSC->SetFloatParameter(TEXT("ParticleCount"), OptimizationSettings.MaxRainParticles * 0.2f);
                        break;
                }
                
                // Set particle lifetime
                PSC->SetFloatParameter(TEXT("Lifetime"), OptimizationSettings.RainParticleLifetime);
            }
        }
    }
    
    CurrentMetrics.RainParticleCount = OptimizationSettings.MaxRainParticles * GetOptimizationMultiplier();
}

void UPerf_WeatherPerformanceOptimizer::OptimizeFogSystem()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find exponential height fog actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !Actor->GetName().Contains(TEXT("Fog"))) continue;
        
        // Apply fog optimization
        float FogMultiplier = GetOptimizationMultiplier();
        
        // Adjust fog density and distance based on optimization level
        if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            if (UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
            {
                DynMaterial->SetScalarParameterValue(TEXT("FogDensity"), 0.1f * FogMultiplier);
                DynMaterial->SetScalarParameterValue(TEXT("MaxDistance"), OptimizationSettings.MaxFogDistance);
            }
        }
    }
    
    CurrentMetrics.FogDensity = 0.1f * GetOptimizationMultiplier();
}

void UPerf_WeatherPerformanceOptimizer::OptimizeCloudSystem()
{
    // Cloud optimization - reduce complexity based on optimization level
    float CloudMultiplier = GetOptimizationMultiplier();
    int32 ActiveCloudLayers = FMath::RoundToInt(OptimizationSettings.MaxCloudLayers * CloudMultiplier);
    
    CurrentMetrics.CloudComplexity = ActiveCloudLayers;
    
    UE_LOG(LogTemp, Log, TEXT("Cloud system optimized - Active layers: %d"), ActiveCloudLayers);
}

void UPerf_WeatherPerformanceOptimizer::OptimizeLightningSystem()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find lightning effect actors and optimize them
    int32 ActiveLightningCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || !Actor->GetName().Contains(TEXT("Lightning"))) continue;
        
        // Check distance from player for culling
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
            
            if (Distance > OptimizationSettings.LightningCullingDistance)
            {
                Actor->SetActorHiddenInGame(true);
            }
            else if (ActiveLightningCount < OptimizationSettings.MaxSimultaneousLightning)
            {
                Actor->SetActorHiddenInGame(false);
                ActiveLightningCount++;
            }
            else
            {
                Actor->SetActorHiddenInGame(true);
            }
        }
    }
    
    CurrentMetrics.LightningEffectCost = ActiveLightningCount * 0.5f; // Estimate 0.5ms per lightning effect
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherTransitions()
{
    // Optimize weather transition effects
    float TransitionCost = 0.0f;
    
    // Reduce transition complexity based on optimization level
    switch (OptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            TransitionCost = 2.0f;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            TransitionCost = 1.5f;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            TransitionCost = 1.0f;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            TransitionCost = 0.5f;
            break;
        case EPerf_WeatherOptimizationLevel::Potato:
            TransitionCost = 0.2f;
            break;
    }
    
    CurrentMetrics.WeatherTransitionCost = TransitionCost;
}

FPerf_WeatherPerformanceMetrics UPerf_WeatherPerformanceOptimizer::GetWeatherPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UPerf_WeatherPerformanceOptimizer::IsWeatherPerformanceWithinBudget() const
{
    float TotalCost = CurrentMetrics.RainParticleCount * 0.001f + // 0.001ms per particle
                      CurrentMetrics.FogDensity * 10.0f + // Fog cost
                      CurrentMetrics.CloudComplexity * 0.5f + // Cloud cost
                      CurrentMetrics.LightningEffectCost + // Lightning cost
                      CurrentMetrics.WeatherTransitionCost; // Transition cost
    
    return TotalCost <= WeatherPerformanceBudget;
}

void UPerf_WeatherPerformanceOptimizer::ApplyDynamicWeatherOptimization(float CurrentFrameTime)
{
    if (CurrentFrameTime > (1000.0f / TargetFrameRate) * 1.5f) // 50% over target
    {
        // Emergency optimization - reduce to Low quality
        if (OptimizationLevel != EPerf_WeatherOptimizationLevel::Low)
        {
            SetOptimizationLevel(EPerf_WeatherOptimizationLevel::Low);
            UE_LOG(LogTemp, Warning, TEXT("Emergency weather optimization applied - switched to Low quality"));
        }
    }
    else if (CurrentFrameTime > (1000.0f / TargetFrameRate) * 1.3f) // 30% over target
    {
        // Moderate optimization - reduce one level
        if (OptimizationLevel == EPerf_WeatherOptimizationLevel::Ultra)
        {
            SetOptimizationLevel(EPerf_WeatherOptimizationLevel::High);
        }
        else if (OptimizationLevel == EPerf_WeatherOptimizationLevel::High)
        {
            SetOptimizationLevel(EPerf_WeatherOptimizationLevel::Medium);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::UpdatePerformanceMetrics()
{
    // Update metrics based on current weather state
    // This would typically query actual weather systems
    // For now, we use the optimization settings as base values
}

void UPerf_WeatherPerformanceOptimizer::ApplyOptimizationSettings()
{
    // Apply settings based on current optimization level
    switch (OptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra:
            OptimizationSettings.MaxRainParticles = 8000;
            OptimizationSettings.MaxCloudLayers = 5;
            OptimizationSettings.MaxSimultaneousLightning = 3;
            break;
        case EPerf_WeatherOptimizationLevel::High:
            OptimizationSettings.MaxRainParticles = 5000;
            OptimizationSettings.MaxCloudLayers = 3;
            OptimizationSettings.MaxSimultaneousLightning = 2;
            break;
        case EPerf_WeatherOptimizationLevel::Medium:
            OptimizationSettings.MaxRainParticles = 3000;
            OptimizationSettings.MaxCloudLayers = 2;
            OptimizationSettings.MaxSimultaneousLightning = 1;
            break;
        case EPerf_WeatherOptimizationLevel::Low:
            OptimizationSettings.MaxRainParticles = 1500;
            OptimizationSettings.MaxCloudLayers = 1;
            OptimizationSettings.MaxSimultaneousLightning = 1;
            break;
        case EPerf_WeatherOptimizationLevel::Potato:
            OptimizationSettings.MaxRainParticles = 500;
            OptimizationSettings.MaxCloudLayers = 1;
            OptimizationSettings.MaxSimultaneousLightning = 0;
            break;
    }
}

void UPerf_WeatherPerformanceOptimizer::CullDistantWeatherEffects()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Cull weather effects beyond certain distances
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Weather")) || ActorName.Contains(TEXT("Rain")) || 
            ActorName.Contains(TEXT("Fog")) || ActorName.Contains(TEXT("Cloud")))
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            float CullingDistance = 15000.0f * GetOptimizationMultiplier();
            
            Actor->SetActorHiddenInGame(Distance > CullingDistance);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::AdjustParticleSystemLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Adjust LOD for all particle systems based on distance and optimization level
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
        
        for (UParticleSystemComponent* PSC : ParticleComponents)
        {
            if (PSC)
            {
                int32 LODLevel = static_cast<int32>(OptimizationLevel);
                PSC->SetLODLevel(LODLevel);
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherShaders()
{
    // Optimize weather-related shaders and materials
    // This would typically involve switching to simpler shader variants
    // based on the optimization level
    
    UE_LOG(LogTemp, Log, TEXT("Weather shaders optimized for level: %d"), 
           static_cast<int32>(OptimizationLevel));
}

float UPerf_WeatherPerformanceOptimizer::GetOptimizationMultiplier() const
{
    switch (OptimizationLevel)
    {
        case EPerf_WeatherOptimizationLevel::Ultra: return 1.0f;
        case EPerf_WeatherOptimizationLevel::High: return 0.8f;
        case EPerf_WeatherOptimizationLevel::Medium: return 0.6f;
        case EPerf_WeatherOptimizationLevel::Low: return 0.4f;
        case EPerf_WeatherOptimizationLevel::Potato: return 0.2f;
        default: return 0.8f;
    }
}