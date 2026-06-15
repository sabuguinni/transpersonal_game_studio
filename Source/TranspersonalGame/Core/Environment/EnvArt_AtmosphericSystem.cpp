#include "EnvArt_AtmosphericSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/VolumetricFog.h"
#include "Sound/AmbientSound.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_AtmosphericSystem::AEnvArt_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    DirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLightComponent"));
    RootComponent = DirectionalLightComponent;

    VolumetricFogComponent = CreateDefaultSubobject<UVolumetricFogComponent>(TEXT("VolumetricFogComponent"));

    SetupDefaultSettings();
}

void AEnvArt_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    UpdateAtmosphericSettings();
}

void AEnvArt_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnvArt_AtmosphericSystem::ApplyGoldenHourLighting()
{
    if (DirectionalLightComponent)
    {
        // Set golden hour rotation
        FRotator GoldenHourRotation(LightingSettings.SunAngle, LightingSettings.SunAzimuth, 0.0f);
        SetActorRotation(GoldenHourRotation);

        // Apply lighting properties
        DirectionalLightComponent->SetLightColor(LightingSettings.SunColor);
        DirectionalLightComponent->SetIntensity(LightingSettings.SunIntensity);
        DirectionalLightComponent->SetCastVolumetricShadow(LightingSettings.bEnableVolumetricShadows);
        DirectionalLightComponent->SetCastShadows(true);
    }
}

void AEnvArt_AtmosphericSystem::SetupVolumetricFog(const FVector& Location, const FVector& Scale)
{
    if (VolumetricFogComponent)
    {
        // Set fog location relative to this actor
        VolumetricFogComponent->SetRelativeLocation(Location);
        VolumetricFogComponent->SetRelativeScale3D(Scale);

        // Apply fog settings
        VolumetricFogComponent->SetScatteringDistribution(FogSettings.ScatteringDistribution);
        VolumetricFogComponent->SetAlbedo(FogSettings.FogAlbedo);
        VolumetricFogComponent->SetExtinctionScale(FogSettings.ExtinctionScale);
        VolumetricFogComponent->SetScatteringScale(FogSettings.ScatteringScale);
    }
}

void AEnvArt_AtmosphericSystem::CreateAudioZone(const FEnvArt_AudioZone& AudioZone)
{
    // Create audio component
    UAudioComponent* NewAudioComponent = CreateDefaultSubobject<UAudioComponent>(*AudioZone.ZoneName);
    if (NewAudioComponent)
    {
        NewAudioComponent->SetRelativeLocation(AudioZone.Location);
        NewAudioComponent->SetVolumeMultiplier(AudioZone.VolumeMultiplier);
        NewAudioComponent->SetPitchMultiplier(AudioZone.PitchMultiplier);
        NewAudioComponent->bAutoActivate = AudioZone.bAutoActivate;

        AudioComponents.Add(NewAudioComponent);
    }
}

void AEnvArt_AtmosphericSystem::UpdateAtmosphericSettings()
{
    ApplyGoldenHourLighting();
    SetupVolumetricFog(FVector(0, 0, 0), FogSettings.FogScale);

    // Create audio zones
    for (const FEnvArt_AudioZone& AudioZone : AudioZones)
    {
        CreateAudioZone(AudioZone);
    }
}

void AEnvArt_AtmosphericSystem::RefreshAtmosphericSystem()
{
    UpdateAtmosphericSettings();
}

void AEnvArt_AtmosphericSystem::InitializeComponents()
{
    // Ensure components are properly initialized
    if (DirectionalLightComponent)
    {
        DirectionalLightComponent->SetMobility(EComponentMobility::Movable);
    }

    if (VolumetricFogComponent)
    {
        VolumetricFogComponent->SetMobility(EComponentMobility::Movable);
    }
}

void AEnvArt_AtmosphericSystem::SetupDefaultSettings()
{
    // Setup default lighting settings for golden hour
    LightingSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    LightingSettings.SunIntensity = 3.5f;
    LightingSettings.SunAngle = -15.0f;
    LightingSettings.SunAzimuth = 45.0f;
    LightingSettings.bEnableVolumetricShadows = true;

    // Setup default fog settings
    FogSettings.FogAlbedo = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    FogSettings.ScatteringDistribution = 0.2f;
    FogSettings.ExtinctionScale = 0.8f;
    FogSettings.ScatteringScale = 1.2f;
    FogSettings.FogScale = FVector(20.0f, 20.0f, 5.0f);

    // Setup default audio zones
    FEnvArt_AudioZone ForestZone;
    ForestZone.ZoneName = "ForestAmbient";
    ForestZone.Location = FVector(2000, 3000, 100);
    ForestZone.VolumeMultiplier = 0.6f;
    ForestZone.PitchMultiplier = 1.0f;
    ForestZone.bAutoActivate = true;
    AudioZones.Add(ForestZone);

    FEnvArt_AudioZone WindZone;
    WindZone.ZoneName = "WindAmbient";
    WindZone.Location = FVector(0, 0, 200);
    WindZone.VolumeMultiplier = 0.4f;
    WindZone.PitchMultiplier = 0.8f;
    WindZone.bAutoActivate = true;
    AudioZones.Add(WindZone);
}