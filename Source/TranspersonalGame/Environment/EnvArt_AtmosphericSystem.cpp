#include "EnvArt_AtmosphericSystem.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEnvArt_AtmosphericSystem::AEnvArt_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle system component
    ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
    ParticleComponent->SetupAttachment(RootComponent);
    ParticleComponent->bAutoActivate = false;

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentIntensity = 1.0f;
    TargetIntensity = 1.0f;
    bIsActive = false;
    EffectRadius = 5000.0f;
    bAutoActivate = true;
}

void AEnvArt_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();

    InitializeParticleSystem();
    InitializeAmbientAudio();

    if (bAutoActivate)
    {
        ActivateAtmosphericEffect();
    }
}

void AEnvArt_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsActive)
    {
        // Smoothly interpolate intensity
        if (FMath::Abs(CurrentIntensity - TargetIntensity) > 0.01f)
        {
            CurrentIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, DeltaTime, 2.0f);
            UpdateParticleParameters();
            UpdateAudioParameters();
        }
    }
}

void AEnvArt_AtmosphericSystem::SetAtmosphericType(EEnvArt_AtmosphericType NewType)
{
    AtmosphericSettings.AtmosphericType = NewType;
    InitializeParticleSystem();
    UpdateParticleParameters();
}

void AEnvArt_AtmosphericSystem::SetIntensity(float NewIntensity)
{
    TargetIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    AtmosphericSettings.Intensity = TargetIntensity;
}

void AEnvArt_AtmosphericSystem::SetWindStrength(float NewWindStrength)
{
    AtmosphericSettings.WindStrength = FMath::Clamp(NewWindStrength, 0.0f, 2.0f);
    UpdateParticleParameters();
}

void AEnvArt_AtmosphericSystem::ActivateAtmosphericEffect()
{
    bIsActive = true;
    
    if (ParticleComponent && ParticleComponent->GetTemplate())
    {
        ParticleComponent->Activate();
    }
    
    if (AmbientAudioComponent && AmbientAudioComponent->GetSound())
    {
        AmbientAudioComponent->Play();
    }
}

void AEnvArt_AtmosphericSystem::DeactivateAtmosphericEffect()
{
    bIsActive = false;
    
    if (ParticleComponent)
    {
        ParticleComponent->Deactivate();
    }
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
}

void AEnvArt_AtmosphericSystem::UpdateForBiome(EBiomeType NewBiome)
{
    BiomeType = NewBiome;
    
    // Set atmospheric type based on biome
    switch (NewBiome)
    {
        case EBiomeType::Savanna:
        case EBiomeType::Desert:
            SetAtmosphericType(EEnvArt_AtmosphericType::DustParticles);
            break;
        case EBiomeType::Swamp:
        case EBiomeType::Forest:
            SetAtmosphericType(EEnvArt_AtmosphericType::MistFog);
            break;
        case EBiomeType::Mountain:
            SetAtmosphericType(EEnvArt_AtmosphericType::VolcanicAsh);
            break;
        default:
            SetAtmosphericType(EEnvArt_AtmosphericType::DustParticles);
            break;
    }
}

void AEnvArt_AtmosphericSystem::UpdateForWeather(EWeatherType WeatherType, float WeatherIntensity)
{
    if (!AtmosphericSettings.bAffectedByWeather)
    {
        return;
    }

    float IntensityMultiplier = 1.0f;
    
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            IntensityMultiplier = 0.3f;
            break;
        case EWeatherType::Cloudy:
            IntensityMultiplier = 0.5f;
            break;
        case EWeatherType::Rainy:
            IntensityMultiplier = 0.1f; // Rain reduces dust/particles
            break;
        case EWeatherType::Stormy:
            IntensityMultiplier = 2.0f; // Storm increases particles
            SetWindStrength(1.5f);
            break;
        case EWeatherType::Foggy:
            if (AtmosphericSettings.AtmosphericType == EEnvArt_AtmosphericType::MistFog)
            {
                IntensityMultiplier = 2.0f;
            }
            break;
    }
    
    SetIntensity(AtmosphericSettings.Intensity * IntensityMultiplier * WeatherIntensity);
}

void AEnvArt_AtmosphericSystem::InitializeParticleSystem()
{
    if (!ParticleComponent)
    {
        return;
    }

    // Load appropriate particle system based on type
    UParticleSystem* ParticleTemplate = nullptr;
    
    switch (AtmosphericSettings.AtmosphericType)
    {
        case EEnvArt_AtmosphericType::DustParticles:
            // Try to load dust particle system
            ParticleTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/EngineMaterials/DefaultParticleSystem"));
            break;
        case EEnvArt_AtmosphericType::MistFog:
            // Try to load mist particle system
            ParticleTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/EngineMaterials/DefaultParticleSystem"));
            break;
        case EEnvArt_AtmosphericType::VolcanicAsh:
            // Try to load ash particle system
            ParticleTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/EngineMaterials/DefaultParticleSystem"));
            break;
        default:
            ParticleTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/EngineMaterials/DefaultParticleSystem"));
            break;
    }
    
    if (ParticleTemplate)
    {
        ParticleComponent->SetTemplate(ParticleTemplate);
    }
}

void AEnvArt_AtmosphericSystem::InitializeAmbientAudio()
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    // Load appropriate ambient sound based on biome
    USoundBase* AmbientSound = nullptr;
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            // Wind through grass sounds
            break;
        case EBiomeType::Forest:
            // Forest ambient sounds
            break;
        case EBiomeType::Swamp:
            // Swamp bubbling and insects
            break;
        case EBiomeType::Desert:
            // Desert wind sounds
            break;
        case EBiomeType::Mountain:
            // Mountain wind and distant rumbles
            break;
    }
    
    if (AmbientSound)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
    }
    
    // Set 3D audio properties
    AmbientAudioComponent->bOverrideAttenuation = true;
    AmbientAudioComponent->AttenuationOverrides.bAttenuate = true;
    AmbientAudioComponent->AttenuationOverrides.FalloffDistance = EffectRadius;
    AmbientAudioComponent->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
}

void AEnvArt_AtmosphericSystem::UpdateParticleParameters()
{
    if (!ParticleComponent || !ParticleComponent->GetTemplate())
    {
        return;
    }

    // Update particle parameters based on current settings
    ParticleComponent->SetFloatParameter(TEXT("Intensity"), CurrentIntensity);
    ParticleComponent->SetFloatParameter(TEXT("WindStrength"), AtmosphericSettings.WindStrength);
    ParticleComponent->SetVectorParameter(TEXT("Scale"), AtmosphericSettings.EffectScale);
}

void AEnvArt_AtmosphericSystem::UpdateAudioParameters()
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    // Update audio volume based on intensity
    float VolumeMultiplier = CurrentIntensity * 0.8f; // Keep it subtle
    AmbientAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
}