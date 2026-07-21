#include "EnvArt_WaterAtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

AEnvArt_WaterAtmosphereSystem::AEnvArt_WaterAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create mist particle component
    MistParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MistParticleComponent"));
    MistParticleComponent->SetupAttachment(RootComponent);

    // Create foam particle component
    FoamParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FoamParticleComponent"));
    FoamParticleComponent->SetupAttachment(RootComponent);

    // Create water audio component
    WaterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WaterAudioComponent"));
    WaterAudioComponent->SetupAttachment(RootComponent);

    // Create water surface component
    WaterSurfaceComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterSurfaceComponent"));
    WaterSurfaceComponent->SetupAttachment(RootComponent);

    // Initialize default settings
    UpdateFrequency = 1.0f;
    bEnableAtmosphericEffects = true;
    bEnableDynamicMist = true;
    LastUpdateTime = 0.0f;

    // Initialize global effect settings
    GlobalEffectSettings.MistDensity = 0.5f;
    GlobalEffectSettings.MistHeight = 200.0f;
    GlobalEffectSettings.SoundVolume = 0.7f;
    GlobalEffectSettings.EffectRadius = 1000.0f;
    GlobalEffectSettings.bEnableRipples = true;
    GlobalEffectSettings.bEnableFoam = true;

    // Initialize biome water data for 5 biomes
    BiomeWaterData.SetNum(5);
    
    // Forest biome water
    BiomeWaterData[0].BiomeType = EBiomeType::Forest;
    BiomeWaterData[0].WaterLocation = FVector(25000, 25000, 80);
    BiomeWaterData[0].WaterTemperature = 18.0f;
    BiomeWaterData[0].FlowRate = 1.2f;
    BiomeWaterData[0].EffectSettings.MistDensity = 0.7f;

    // Mountain biome water
    BiomeWaterData[1].BiomeType = EBiomeType::Mountain;
    BiomeWaterData[1].WaterLocation = FVector(75000, 75000, 120);
    BiomeWaterData[1].WaterTemperature = 12.0f;
    BiomeWaterData[1].FlowRate = 2.0f;
    BiomeWaterData[1].EffectSettings.MistDensity = 0.9f;

    // Plains biome water
    BiomeWaterData[2].BiomeType = EBiomeType::Plains;
    BiomeWaterData[2].WaterLocation = FVector(50000, 50000, 100);
    BiomeWaterData[2].WaterTemperature = 22.0f;
    BiomeWaterData[2].FlowRate = 0.8f;
    BiomeWaterData[2].EffectSettings.MistDensity = 0.3f;

    // Wetland biome water
    BiomeWaterData[3].BiomeType = EBiomeType::Wetland;
    BiomeWaterData[3].WaterLocation = FVector(25000, 75000, 90);
    BiomeWaterData[3].WaterTemperature = 25.0f;
    BiomeWaterData[3].FlowRate = 0.5f;
    BiomeWaterData[3].EffectSettings.MistDensity = 1.0f;

    // Desert biome water (rare oasis)
    BiomeWaterData[4].BiomeType = EBiomeType::Desert;
    BiomeWaterData[4].WaterLocation = FVector(75000, 25000, 110);
    BiomeWaterData[4].WaterTemperature = 28.0f;
    BiomeWaterData[4].FlowRate = 0.2f;
    BiomeWaterData[4].EffectSettings.MistDensity = 0.1f;
}

void AEnvArt_WaterAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWaterEffects();
    
    // Set up timer for periodic updates
    if (UpdateFrequency > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &AEnvArt_WaterAtmosphereSystem::InitializeWaterEffects,
            UpdateFrequency,
            true
        );
    }
}

void AEnvArt_WaterAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableAtmosphericEffects)
    {
        UpdateAtmosphericEffects(DeltaTime);
    }

    LastUpdateTime += DeltaTime;
}

void AEnvArt_WaterAtmosphereSystem::InitializeWaterEffects()
{
    if (!bEnableAtmosphericEffects)
    {
        return;
    }

    // Create atmospheric effects for each biome
    for (const FEnvArt_WaterBiomeData& BiomeData : BiomeWaterData)
    {
        CreateMistEffect(BiomeData.WaterLocation, BiomeData.EffectSettings.MistDensity);
        CreateWaterSound(BiomeData.WaterLocation, BiomeData.EffectSettings.SoundVolume);
    }

    UE_LOG(LogTemp, Log, TEXT("Water atmospheric effects initialized for %d biomes"), BiomeWaterData.Num());
}

void AEnvArt_WaterAtmosphereSystem::UpdateAtmosphericEffects(float DeltaTime)
{
    if (bEnableDynamicMist)
    {
        UpdateMistEffects(DeltaTime);
    }

    UpdateWaterSounds(DeltaTime);
    
    // Clean up inactive effects every 5 seconds
    if (FMath::Fmod(LastUpdateTime, 5.0f) < DeltaTime)
    {
        CleanupInactiveEffects();
    }
}

void AEnvArt_WaterAtmosphereSystem::SetBiomeWaterEffect(EBiomeType BiomeType, const FEnvArt_WaterEffectSettings& Settings)
{
    for (FEnvArt_WaterBiomeData& BiomeData : BiomeWaterData)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            BiomeData.EffectSettings = Settings;
            break;
        }
    }
}

FEnvArt_WaterEffectSettings AEnvArt_WaterAtmosphereSystem::GetBiomeWaterEffect(EBiomeType BiomeType) const
{
    for (const FEnvArt_WaterBiomeData& BiomeData : BiomeWaterData)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData.EffectSettings;
        }
    }
    
    return GlobalEffectSettings;
}

void AEnvArt_WaterAtmosphereSystem::CreateMistEffect(const FVector& Location, float Intensity)
{
    if (!MistParticleComponent || !bEnableDynamicMist)
    {
        return;
    }

    // Create new mist particle component
    UParticleSystemComponent* NewMist = NewObject<UParticleSystemComponent>(this);
    if (NewMist)
    {
        NewMist->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        NewMist->SetWorldLocation(Location + FVector(0, 0, GlobalEffectSettings.MistHeight));
        
        // Set mist intensity based on biome settings
        if (NewMist->GetTemplate())
        {
            NewMist->SetFloatParameter(FName("Intensity"), Intensity);
            NewMist->SetFloatParameter(FName("Density"), Intensity * GlobalEffectSettings.MistDensity);
        }
        
        NewMist->Activate();
        ActiveMistEffects.Add(NewMist);
        
        UE_LOG(LogTemp, Log, TEXT("Created mist effect at location: %s with intensity: %f"), 
               *Location.ToString(), Intensity);
    }
}

void AEnvArt_WaterAtmosphereSystem::CreateWaterSound(const FVector& Location, float Volume)
{
    if (!WaterAudioComponent)
    {
        return;
    }

    // Create new audio component for water sounds
    UAudioComponent* NewWaterSound = NewObject<UAudioComponent>(this);
    if (NewWaterSound)
    {
        NewWaterSound->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
        NewWaterSound->SetWorldLocation(Location);
        NewWaterSound->SetVolumeMultiplier(Volume * GlobalEffectSettings.SoundVolume);
        
        // Set 3D audio properties
        NewWaterSound->bAllowSpatialization = true;
        NewWaterSound->AttenuationSettings = nullptr; // Use default attenuation
        
        NewWaterSound->Play();
        ActiveWaterSounds.Add(NewWaterSound);
        
        UE_LOG(LogTemp, Log, TEXT("Created water sound at location: %s with volume: %f"), 
               *Location.ToString(), Volume);
    }
}

void AEnvArt_WaterAtmosphereSystem::UpdateWaterTemperature(float NewTemperature)
{
    for (FEnvArt_WaterBiomeData& BiomeData : BiomeWaterData)
    {
        BiomeData.WaterTemperature = NewTemperature;
        
        // Adjust mist density based on temperature
        float TemperatureFactor = FMath::Clamp((NewTemperature - 10.0f) / 20.0f, 0.1f, 1.5f);
        BiomeData.EffectSettings.MistDensity *= TemperatureFactor;
    }
}

void AEnvArt_WaterAtmosphereSystem::SetFlowRate(float NewFlowRate)
{
    for (FEnvArt_WaterBiomeData& BiomeData : BiomeWaterData)
    {
        BiomeData.FlowRate = FMath::Clamp(NewFlowRate, 0.1f, 3.0f);
        
        // Adjust sound volume based on flow rate
        float FlowFactor = FMath::Clamp(BiomeData.FlowRate / 2.0f, 0.3f, 1.2f);
        BiomeData.EffectSettings.SoundVolume *= FlowFactor;
    }
}

void AEnvArt_WaterAtmosphereSystem::UpdateMistEffects(float DeltaTime)
{
    // Update existing mist effects based on environmental conditions
    for (UParticleSystemComponent* MistEffect : ActiveMistEffects)
    {
        if (MistEffect && IsValid(MistEffect))
        {
            // Simulate wind effect on mist
            float WindStrength = FMath::Sin(LastUpdateTime * 0.5f) * 0.3f + 0.7f;
            MistEffect->SetFloatParameter(FName("WindStrength"), WindStrength);
            
            // Vary mist density over time
            float DensityVariation = FMath::Sin(LastUpdateTime * 0.2f) * 0.2f + 0.8f;
            MistEffect->SetFloatParameter(FName("DensityVariation"), DensityVariation);
        }
    }
}

void AEnvArt_WaterAtmosphereSystem::UpdateWaterSounds(float DeltaTime)
{
    // Update water sound volumes based on distance and environmental factors
    for (UAudioComponent* WaterSound : ActiveWaterSounds)
    {
        if (WaterSound && IsValid(WaterSound))
        {
            // Simulate natural volume variation
            float VolumeVariation = FMath::Sin(LastUpdateTime * 0.3f) * 0.1f + 0.9f;
            float CurrentVolume = WaterSound->VolumeMultiplier * VolumeVariation;
            WaterSound->SetVolumeMultiplier(FMath::Clamp(CurrentVolume, 0.1f, 1.0f));
        }
    }
}

void AEnvArt_WaterAtmosphereSystem::CleanupInactiveEffects()
{
    // Remove inactive mist effects
    ActiveMistEffects.RemoveAll([](UParticleSystemComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });

    // Remove inactive water sounds
    ActiveWaterSounds.RemoveAll([](UAudioComponent* Sound)
    {
        return !Sound || !IsValid(Sound) || !Sound->IsPlaying();
    });

    UE_LOG(LogTemp, Log, TEXT("Cleanup completed. Active mist effects: %d, Active water sounds: %d"), 
           ActiveMistEffects.Num(), ActiveWaterSounds.Num());
}