#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AVFX_ParticleManager::AVFX_ParticleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize VFX configuration
    MaxActiveEffects = 50.0f;
    EffectCullDistance = 10000.0f; // 100 meters

    // Initialize impact configuration
    ImpactConfig.MinImpactForce = 100.0f;
    ImpactConfig.MaxParticleCount = 500.0f;
}

void AVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXSystems();
    SetupEffectCleanupTimer();
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Initialized with %d biome effects"), BiomeAmbientEffects.Num());
}

void AVFX_ParticleManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllEffects();
    
    if (GetWorld() && EffectCleanupTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(EffectCleanupTimer);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AVFX_ParticleManager::InitializeVFXSystems()
{
    // Initialize biome-specific ambient effects
    FVFX_ParticleEffectData SavannaAmbient;
    SavannaAmbient.Duration = -1.0f; // Infinite duration for ambient
    SavannaAmbient.IntensityMultiplier = 0.5f;
    SavannaAmbient.bAutoDestroy = false;
    BiomeAmbientEffects.Add(EBiomeType::Savanna, SavannaAmbient);

    FVFX_ParticleEffectData ForestAmbient;
    ForestAmbient.Duration = -1.0f;
    ForestAmbient.IntensityMultiplier = 0.7f;
    ForestAmbient.bAutoDestroy = false;
    BiomeAmbientEffects.Add(EBiomeType::Forest, ForestAmbient);

    FVFX_ParticleEffectData DesertAmbient;
    DesertAmbient.Duration = -1.0f;
    DesertAmbient.IntensityMultiplier = 0.3f;
    DesertAmbient.bAutoDestroy = false;
    BiomeAmbientEffects.Add(EBiomeType::Desert, DesertAmbient);

    FVFX_ParticleEffectData SwampAmbient;
    SwampAmbient.Duration = -1.0f;
    SwampAmbient.IntensityMultiplier = 0.8f;
    SwampAmbient.bAutoDestroy = false;
    BiomeAmbientEffects.Add(EBiomeType::Swamp, SwampAmbient);

    FVFX_ParticleEffectData MountainAmbient;
    MountainAmbient.Duration = -1.0f;
    MountainAmbient.IntensityMultiplier = 0.4f;
    MountainAmbient.bAutoDestroy = false;
    BiomeAmbientEffects.Add(EBiomeType::Mountain, MountainAmbient);
}

void AVFX_ParticleManager::SetupEffectCleanupTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EffectCleanupTimer,
            this,
            &AVFX_ParticleManager::CleanupExpiredEffects,
            2.0f, // Every 2 seconds
            true
        );
    }
}

UNiagaraComponent* AVFX_ParticleManager::SpawnParticleEffect(const FVFX_ParticleEffectData& EffectData)
{
    if (!EffectData.NiagaraSystem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Invalid Niagara system in effect data"));
        return nullptr;
    }

    // Check if we're at the effect limit
    if (ActiveParticleComponents.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Max active effects reached (%d)"), (int32)MaxActiveEffects);
        return nullptr;
    }

    // Check distance culling
    if (!IsEffectInRange(EffectData.SpawnLocation, EffectCullDistance))
    {
        return nullptr;
    }

    // Load the Niagara system
    UNiagaraSystem* NiagaraSystem = EffectData.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleManager: Failed to load Niagara system"));
        return nullptr;
    }

    // Create and configure the component
    UNiagaraComponent* NewComponent = CreateNiagaraComponent(NiagaraSystem);
    if (NewComponent)
    {
        NewComponent->SetWorldLocationAndRotation(EffectData.SpawnLocation, EffectData.SpawnRotation);
        ConfigureEffectParameters(NewComponent, EffectData);
        
        ActiveParticleComponents.Add(NewComponent);
        
        // Add to active effects for tracking
        FVFX_ParticleEffectData TrackedEffect = EffectData;
        ActiveEffects.Add(TrackedEffect);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect at %s"), 
               *EffectData.SpawnLocation.ToString());
    }

    return NewComponent;
}

void AVFX_ParticleManager::SpawnImpactEffect(FVector ImpactLocation, FVector ImpactNormal, float ImpactForce, EImpactType ImpactType)
{
    if (ImpactForce < ImpactConfig.MinImpactForce)
    {
        return;
    }

    FVFX_ParticleEffectData ImpactEffect;
    ImpactEffect.SpawnLocation = ImpactLocation;
    ImpactEffect.SpawnRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    ImpactEffect.Duration = 3.0f;
    ImpactEffect.IntensityMultiplier = FMath::Clamp(ImpactForce / 1000.0f, 0.1f, 2.0f);

    switch (ImpactType)
    {
        case EImpactType::Ground:
            ImpactEffect.NiagaraSystem = ImpactConfig.DustEffect;
            break;
        case EImpactType::Flesh:
            ImpactEffect.NiagaraSystem = ImpactConfig.BloodEffect;
            break;
        case EImpactType::Stone:
            ImpactEffect.NiagaraSystem = ImpactConfig.SparkEffect;
            break;
        default:
            ImpactEffect.NiagaraSystem = ImpactConfig.DustEffect;
            break;
    }

    SpawnParticleEffect(ImpactEffect);
}

void AVFX_ParticleManager::SpawnDinosaurFootstepEffect(FVector FootLocation, float DinosaurMass, ESurfaceType SurfaceType)
{
    FVFX_ParticleEffectData FootstepEffect;
    FootstepEffect.SpawnLocation = FootLocation;
    FootstepEffect.SpawnRotation = FRotator::ZeroRotator;
    FootstepEffect.Duration = 2.0f;
    
    // Scale intensity based on dinosaur mass
    FootstepEffect.IntensityMultiplier = FMath::Clamp(DinosaurMass / 5000.0f, 0.2f, 3.0f);

    // Use dust effect for all surface types (can be expanded)
    FootstepEffect.NiagaraSystem = ImpactConfig.DustEffect;

    SpawnParticleEffect(FootstepEffect);
}

void AVFX_ParticleManager::SpawnWeatherEffect(EWeatherType WeatherType, FVector Location, float Intensity)
{
    FVFX_ParticleEffectData WeatherEffect;
    WeatherEffect.SpawnLocation = Location;
    WeatherEffect.SpawnRotation = FRotator::ZeroRotator;
    WeatherEffect.Duration = -1.0f; // Infinite for weather
    WeatherEffect.IntensityMultiplier = Intensity;
    WeatherEffect.bAutoDestroy = false;

    // Weather effects would use specific Niagara systems (to be created)
    // For now, use placeholder dust effect
    WeatherEffect.NiagaraSystem = ImpactConfig.DustEffect;

    SpawnParticleEffect(WeatherEffect);
}

void AVFX_ParticleManager::SpawnFireEffect(FVector FireLocation, float FireIntensity, float Duration)
{
    FVFX_ParticleEffectData FireEffect;
    FireEffect.SpawnLocation = FireLocation;
    FireEffect.SpawnRotation = FRotator::ZeroRotator;
    FireEffect.Duration = Duration;
    FireEffect.IntensityMultiplier = FireIntensity;

    // Fire effect would use specific Niagara system (to be created)
    // For now, use placeholder dust effect
    FireEffect.NiagaraSystem = ImpactConfig.DustEffect;

    SpawnParticleEffect(FireEffect);
}

void AVFX_ParticleManager::SpawnBloodEffect(FVector BloodLocation, FVector BloodDirection, float BloodAmount)
{
    FVFX_ParticleEffectData BloodEffect;
    BloodEffect.SpawnLocation = BloodLocation;
    BloodEffect.SpawnRotation = FRotationMatrix::MakeFromZ(BloodDirection).Rotator();
    BloodEffect.Duration = 5.0f;
    BloodEffect.IntensityMultiplier = FMath::Clamp(BloodAmount, 0.1f, 2.0f);
    BloodEffect.NiagaraSystem = ImpactConfig.BloodEffect;

    SpawnParticleEffect(BloodEffect);
}

void AVFX_ParticleManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveParticleComponents[i];
        if (!Component || !Component->IsValidLowLevel() || !Component->IsActive())
        {
            ActiveParticleComponents.RemoveAt(i);
            if (i < ActiveEffects.Num())
            {
                ActiveEffects.RemoveAt(i);
            }
        }
    }

    // Remove expired effects based on duration
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (ActiveEffects[i].Duration > 0.0f)
        {
            ActiveEffects[i].Duration -= 2.0f; // Timer interval
            if (ActiveEffects[i].Duration <= 0.0f)
            {
                if (i < ActiveParticleComponents.Num() && ActiveParticleComponents[i])
                {
                    ActiveParticleComponents[i]->DestroyComponent();
                }
                ActiveParticleComponents.RemoveAt(i);
                ActiveEffects.RemoveAt(i);
            }
        }
    }
}

void AVFX_ParticleManager::SetBiomeAmbientEffect(EBiomeType BiomeType, bool bEnable)
{
    if (!BiomeAmbientEffects.Contains(BiomeType))
    {
        return;
    }

    FVFX_ParticleEffectData* AmbientEffect = BiomeAmbientEffects.Find(BiomeType);
    if (AmbientEffect)
    {
        if (bEnable)
        {
            AmbientEffect->SpawnLocation = GetActorLocation();
            SpawnParticleEffect(*AmbientEffect);
        }
        else
        {
            // Stop ambient effects for this biome
            // Implementation would require tracking biome-specific components
        }
    }
}

void AVFX_ParticleManager::UpdateEffectIntensity(UNiagaraComponent* EffectComponent, float NewIntensity)
{
    if (!EffectComponent)
    {
        return;
    }

    // Update Niagara parameters for intensity
    EffectComponent->SetFloatParameter(TEXT("Intensity"), NewIntensity);
    EffectComponent->SetFloatParameter(TEXT("SpawnRate"), NewIntensity * 100.0f);
}

void AVFX_ParticleManager::StopAllEffects()
{
    for (UNiagaraComponent* Component : ActiveParticleComponents)
    {
        if (Component && Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }

    ActiveParticleComponents.Empty();
    ActiveEffects.Empty();
}

int32 AVFX_ParticleManager::GetActiveEffectCount() const
{
    return ActiveParticleComponents.Num();
}

bool AVFX_ParticleManager::IsEffectInRange(FVector EffectLocation, float MaxDistance) const
{
    if (!GetWorld())
    {
        return false;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return true; // If no player, don't cull
    }

    float DistanceSquared = FVector::DistSquared(PlayerPawn->GetActorLocation(), EffectLocation);
    return DistanceSquared <= (MaxDistance * MaxDistance);
}

void AVFX_ParticleManager::OptimizeEffectsForPerformance()
{
    // Remove effects that are too far from player
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        if (i < ActiveEffects.Num())
        {
            if (!IsEffectInRange(ActiveEffects[i].SpawnLocation, EffectCullDistance))
            {
                if (ActiveParticleComponents[i])
                {
                    ActiveParticleComponents[i]->DestroyComponent();
                }
                ActiveParticleComponents.RemoveAt(i);
                ActiveEffects.RemoveAt(i);
            }
        }
    }

    // Reduce intensity of distant effects
    if (GetWorld())
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            for (int32 i = 0; i < ActiveParticleComponents.Num() && i < ActiveEffects.Num(); i++)
            {
                float Distance = FVector::Dist(PlayerLocation, ActiveEffects[i].SpawnLocation);
                float IntensityScale = FMath::Clamp(1.0f - (Distance / EffectCullDistance), 0.1f, 1.0f);
                
                if (ActiveParticleComponents[i])
                {
                    UpdateEffectIntensity(ActiveParticleComponents[i], 
                                        ActiveEffects[i].IntensityMultiplier * IntensityScale);
                }
            }
        }
    }
}

UNiagaraComponent* AVFX_ParticleManager::CreateNiagaraComponent(UNiagaraSystem* NiagaraSystem)
{
    if (!NiagaraSystem)
    {
        return nullptr;
    }

    UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(this);
    if (NewComponent)
    {
        NewComponent->SetAsset(NiagaraSystem);
        NewComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        NewComponent->Activate();
    }

    return NewComponent;
}

void AVFX_ParticleManager::ConfigureEffectParameters(UNiagaraComponent* Component, const FVFX_ParticleEffectData& EffectData)
{
    if (!Component)
    {
        return;
    }

    // Set common Niagara parameters
    Component->SetFloatParameter(TEXT("Intensity"), EffectData.IntensityMultiplier);
    Component->SetFloatParameter(TEXT("SpawnRate"), EffectData.IntensityMultiplier * 100.0f);
    Component->SetVectorParameter(TEXT("SpawnLocation"), EffectData.SpawnLocation);
    
    if (EffectData.Duration > 0.0f)
    {
        Component->SetFloatParameter(TEXT("Duration"), EffectData.Duration);
    }
}

bool AVFX_ParticleManager::ShouldCullEffect(const FVFX_ParticleEffectData& EffectData) const
{
    return !IsEffectInRange(EffectData.SpawnLocation, EffectCullDistance);
}

void AVFX_ParticleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic optimization
    static float OptimizationTimer = 0.0f;
    OptimizationTimer += DeltaTime;
    
    if (OptimizationTimer >= 5.0f) // Optimize every 5 seconds
    {
        OptimizeEffectsForPerformance();
        OptimizationTimer = 0.0f;
    }
}