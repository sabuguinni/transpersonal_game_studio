#include "Perf_WeatherPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"

UPerf_WeatherPerformanceOptimizer::UPerf_WeatherPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentPerformanceLevel = EPerf_WeatherPerformanceLevel::High;
    PerformanceUpdateInterval = 1.0f;
    LastPerformanceUpdate = 0.0f;
    WeatherPerformanceBudget = 5.0f; // 5ms budget for weather effects
    bAutoAdjustQuality = true;
    
    InitializePerformanceSettings();
}

void UPerf_WeatherPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance monitoring
    UpdatePerformanceMetrics();
    
    // Find and register existing weather components
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                
                for (UNiagaraComponent* Component : NiagaraComponents)
                {
                    if (Component && Component->GetAsset())
                    {
                        FString AssetName = Component->GetAsset()->GetName();
                        if (AssetName.Contains("Rain") || AssetName.Contains("Snow") || 
                            AssetName.Contains("Fog") || AssetName.Contains("Weather"))
                        {
                            RegisterWeatherComponent(Component);
                        }
                    }
                }
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastPerformanceUpdate += DeltaTime;
    
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bAutoAdjustQuality)
        {
            float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
            AdjustWeatherQuality(CurrentFrameTime);
        }
        
        OptimizeWeatherEffects();
        CullDistantWeatherEffects();
        CleanupInvalidComponents();
        
        LastPerformanceUpdate = 0.0f;
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeWeatherEffects()
{
    for (int32 i = TrackedWeatherComponents.Num() - 1; i >= 0; --i)
    {
        if (TrackedWeatherComponents[i].IsValid())
        {
            UNiagaraComponent* Component = TrackedWeatherComponents[i].Get();
            OptimizeRainSystem(Component);
            
            // Update LOD based on distance to player
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    float Distance = FVector::Dist(Component->GetComponentLocation(), PlayerPawn->GetActorLocation());
                    UpdateWeatherLOD(Distance);
                }
            }
        }
        else
        {
            TrackedWeatherComponents.RemoveAt(i);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::SetWeatherPerformanceLevel(EPerf_WeatherPerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;
    PerformanceSettings = GetSettingsForLevel(Level);
    ApplyPerformanceSettings(PerformanceSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Weather Performance Level set to: %d"), (int32)Level);
}

void UPerf_WeatherPerformanceOptimizer::UpdateWeatherLOD(float DistanceToPlayer)
{
    for (TWeakObjectPtr<UNiagaraComponent>& ComponentPtr : TrackedWeatherComponents)
    {
        if (ComponentPtr.IsValid())
        {
            UNiagaraComponent* Component = ComponentPtr.Get();
            
            // Adjust particle count based on distance
            float LODScale = 1.0f;
            if (DistanceToPlayer > 10000.0f)
            {
                LODScale = 0.25f; // 25% particles at long distance
            }
            else if (DistanceToPlayer > 5000.0f)
            {
                LODScale = 0.5f; // 50% particles at medium distance
            }
            else if (DistanceToPlayer > 2000.0f)
            {
                LODScale = 0.75f; // 75% particles at close distance
            }
            
            // Apply LOD scaling
            Component->SetFloatParameter("LODScale", LODScale);
            
            // Disable component if too far
            if (DistanceToPlayer > PerformanceSettings.MaxWeatherDrawDistance)
            {
                Component->SetVisibility(false);
            }
            else
            {
                Component->SetVisibility(true);
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::OptimizeRainSystem(UNiagaraComponent* RainComponent)
{
    if (!RainComponent || !RainComponent->GetAsset())
    {
        return;
    }
    
    // Adjust particle count based on performance settings
    int32 ParticleCount = FMath::Min(PerformanceSettings.MaxRainParticles, 10000);
    RainComponent->SetIntParameter("MaxParticles", ParticleCount);
    
    // Adjust update frequency
    RainComponent->SetFloatParameter("UpdateFrequency", PerformanceSettings.WeatherUpdateFrequency);
    
    // Enable/disable expensive features
    RainComponent->SetBoolParameter("EnableCollision", CurrentPerformanceLevel >= EPerf_WeatherPerformanceLevel::High);
    RainComponent->SetBoolParameter("EnableShadows", PerformanceSettings.bEnableWeatherShadows);
}

void UPerf_WeatherPerformanceOptimizer::OptimizeFogSystem()
{
    if (UWorld* World = GetWorld())
    {
        // Find and optimize fog volumes
        int32 FogVolumeCount = 0;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetClass()->GetName().Contains("Fog"))
            {
                FogVolumeCount++;
                
                // Disable fog volumes beyond the limit
                if (FogVolumeCount > PerformanceSettings.MaxFogVolumes)
                {
                    Actor->SetActorHiddenInGame(true);
                }
                else
                {
                    Actor->SetActorHiddenInGame(false);
                    
                    // Adjust fog quality
                    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
                    {
                        if (UMaterialInterface* Material = MeshComp->GetMaterial(0))
                        {
                            // Adjust volumetric fog settings based on performance level
                            bool bEnableVolumetric = PerformanceSettings.bEnableVolumetricFog && 
                                                   (CurrentPerformanceLevel >= EPerf_WeatherPerformanceLevel::Medium);
                            
                            // This would typically set material parameters
                            // MeshComp->SetScalarParameterValueOnMaterials("VolumetricFogEnabled", bEnableVolumetric ? 1.0f : 0.0f);
                        }
                    }
                }
            }
        }
    }
}

float UPerf_WeatherPerformanceOptimizer::GetWeatherPerformanceImpact() const
{
    float TotalImpact = 0.0f;
    
    for (const TWeakObjectPtr<UNiagaraComponent>& ComponentPtr : TrackedWeatherComponents)
    {
        if (ComponentPtr.IsValid())
        {
            UNiagaraComponent* Component = ComponentPtr.Get();
            if (Component->IsActive())
            {
                // Estimate performance impact based on particle count and complexity
                int32 ParticleCount = 1000; // Default estimate
                Component->GetIntParameter("MaxParticles", ParticleCount);
                
                // Simple heuristic: 1ms per 1000 particles
                TotalImpact += ParticleCount / 1000.0f;
            }
        }
    }
    
    return TotalImpact;
}

void UPerf_WeatherPerformanceOptimizer::CullDistantWeatherEffects()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            for (TWeakObjectPtr<UNiagaraComponent>& ComponentPtr : TrackedWeatherComponents)
            {
                if (ComponentPtr.IsValid())
                {
                    UNiagaraComponent* Component = ComponentPtr.Get();
                    float Distance = FVector::Dist(Component->GetComponentLocation(), PlayerLocation);
                    
                    // Cull effects beyond draw distance
                    if (Distance > PerformanceSettings.MaxWeatherDrawDistance)
                    {
                        Component->SetVisibility(false);
                        Component->Deactivate();
                    }
                    else if (!Component->IsVisible())
                    {
                        Component->SetVisibility(true);
                        Component->Activate();
                    }
                }
            }
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::AdjustWeatherQuality(float FrameTime)
{
    // Target frame times: 16.67ms (60fps), 33.33ms (30fps)
    float TargetFrameTime = 16.67f; // 60fps target
    
    if (FrameTime > TargetFrameTime + 5.0f) // If we're 5ms over budget
    {
        // Reduce quality
        if (CurrentPerformanceLevel > EPerf_WeatherPerformanceLevel::Minimal)
        {
            EPerf_WeatherPerformanceLevel NewLevel = (EPerf_WeatherPerformanceLevel)((int32)CurrentPerformanceLevel - 1);
            SetWeatherPerformanceLevel(NewLevel);
        }
    }
    else if (FrameTime < TargetFrameTime - 5.0f) // If we have 5ms headroom
    {
        // Increase quality
        if (CurrentPerformanceLevel < EPerf_WeatherPerformanceLevel::Ultra)
        {
            EPerf_WeatherPerformanceLevel NewLevel = (EPerf_WeatherPerformanceLevel)((int32)CurrentPerformanceLevel + 1);
            SetWeatherPerformanceLevel(NewLevel);
        }
    }
}

void UPerf_WeatherPerformanceOptimizer::InitializePerformanceSettings()
{
    PerformanceSettings = GetSettingsForLevel(CurrentPerformanceLevel);
}

void UPerf_WeatherPerformanceOptimizer::UpdatePerformanceMetrics()
{
    // Update internal performance tracking
    float CurrentImpact = GetWeatherPerformanceImpact();
    
    if (CurrentImpact > WeatherPerformanceBudget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weather performance impact (%.2fms) exceeds budget (%.2fms)"), 
               CurrentImpact, WeatherPerformanceBudget);
    }
}

FPerf_WeatherPerformanceSettings UPerf_WeatherPerformanceOptimizer::GetSettingsForLevel(EPerf_WeatherPerformanceLevel Level)
{
    FPerf_WeatherPerformanceSettings Settings;
    
    switch (Level)
    {
        case EPerf_WeatherPerformanceLevel::Ultra:
            Settings.MaxRainParticles = 20000;
            Settings.MaxFogVolumes = 10;
            Settings.WeatherUpdateFrequency = 2.0f;
            Settings.bEnableVolumetricFog = true;
            Settings.bEnableWeatherShadows = true;
            Settings.MaxWeatherDrawDistance = 100000.0f;
            break;
            
        case EPerf_WeatherPerformanceLevel::High:
            Settings.MaxRainParticles = 15000;
            Settings.MaxFogVolumes = 8;
            Settings.WeatherUpdateFrequency = 1.5f;
            Settings.bEnableVolumetricFog = true;
            Settings.bEnableWeatherShadows = true;
            Settings.MaxWeatherDrawDistance = 75000.0f;
            break;
            
        case EPerf_WeatherPerformanceLevel::Medium:
            Settings.MaxRainParticles = 10000;
            Settings.MaxFogVolumes = 5;
            Settings.WeatherUpdateFrequency = 1.0f;
            Settings.bEnableVolumetricFog = true;
            Settings.bEnableWeatherShadows = false;
            Settings.MaxWeatherDrawDistance = 50000.0f;
            break;
            
        case EPerf_WeatherPerformanceLevel::Low:
            Settings.MaxRainParticles = 5000;
            Settings.MaxFogVolumes = 3;
            Settings.WeatherUpdateFrequency = 0.5f;
            Settings.bEnableVolumetricFog = false;
            Settings.bEnableWeatherShadows = false;
            Settings.MaxWeatherDrawDistance = 25000.0f;
            break;
            
        case EPerf_WeatherPerformanceLevel::Minimal:
            Settings.MaxRainParticles = 1000;
            Settings.MaxFogVolumes = 1;
            Settings.WeatherUpdateFrequency = 0.25f;
            Settings.bEnableVolumetricFog = false;
            Settings.bEnableWeatherShadows = false;
            Settings.MaxWeatherDrawDistance = 10000.0f;
            break;
    }
    
    return Settings;
}

void UPerf_WeatherPerformanceOptimizer::ApplyPerformanceSettings(const FPerf_WeatherPerformanceSettings& Settings)
{
    PerformanceSettings = Settings;
    
    // Apply settings to all tracked components
    for (TWeakObjectPtr<UNiagaraComponent>& ComponentPtr : TrackedWeatherComponents)
    {
        if (ComponentPtr.IsValid())
        {
            OptimizeRainSystem(ComponentPtr.Get());
        }
    }
    
    OptimizeFogSystem();
}

void UPerf_WeatherPerformanceOptimizer::RegisterWeatherComponent(UNiagaraComponent* Component)
{
    if (Component && !TrackedWeatherComponents.Contains(Component))
    {
        TrackedWeatherComponents.Add(Component);
        OptimizeRainSystem(Component);
    }
}

void UPerf_WeatherPerformanceOptimizer::UnregisterWeatherComponent(UNiagaraComponent* Component)
{
    TrackedWeatherComponents.RemoveAll([Component](const TWeakObjectPtr<UNiagaraComponent>& Ptr)
    {
        return Ptr.Get() == Component;
    });
}

void UPerf_WeatherPerformanceOptimizer::CleanupInvalidComponents()
{
    TrackedWeatherComponents.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& Ptr)
    {
        return !Ptr.IsValid();
    });
}