#include "EnvArt_AtmosphericManager.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create fog component
    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FogComponent"));
    RootComponent = FogComponent;

    // Create sky light component
    SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLightComponent"));
    SkyLightComponent->SetupAttachment(RootComponent);

    // Initialize default values
    TimeOfDay = 12.0f; // Noon
    bEnableDynamicWeather = true;
    WeatherTransitionSpeed = 1.0f;
    CurrentBiome = EBiomeType::Savanna;
    AtmosphereBlendTime = 0.0f;
    bIsTransitioning = false;

    InitializeBiomeSettings();
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial atmosphere based on current biome
    SetBiomeAtmosphere(CurrentBiome);
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicWeather)
    {
        // Update time-based atmospheric changes
        UpdateTimeOfDay(TimeOfDay + DeltaTime * 0.1f); // Slow time progression
    }

    if (bIsTransitioning)
    {
        AtmosphereBlendTime += DeltaTime * WeatherTransitionSpeed;
        if (AtmosphereBlendTime >= 1.0f)
        {
            AtmosphereBlendTime = 1.0f;
            bIsTransitioning = false;
        }
    }
}

void AEnvArt_AtmosphericManager::SetBiomeAtmosphere(EBiomeType BiomeType)
{
    if (BiomeAtmosphereSettings.Contains(BiomeType))
    {
        CurrentBiome = BiomeType;
        const FEnvArt_BiomeAtmosphere& Settings = BiomeAtmosphereSettings[BiomeType];
        
        UpdateFogSettings(Settings);
        UpdateSkyLightSettings(Settings);
        
        bIsTransitioning = true;
        AtmosphereBlendTime = 0.0f;
    }
}

void AEnvArt_AtmosphericManager::UpdateTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Fmod(NewTimeOfDay, 24.0f);
    
    // Adjust atmosphere based on time of day
    if (FogComponent)
    {
        float TimeMultiplier = 1.0f;
        if (TimeOfDay < 6.0f || TimeOfDay > 18.0f) // Night
        {
            TimeMultiplier = 0.3f;
        }
        else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f) // Dawn/Dusk
        {
            TimeMultiplier = 0.7f;
        }
        
        FogComponent->SetFogDensity(FogComponent->GetFogDensity() * TimeMultiplier);
    }
}

void AEnvArt_AtmosphericManager::SetWeatherIntensity(float Intensity)
{
    if (FogComponent)
    {
        float BaseDensity = BiomeAtmosphereSettings.Contains(CurrentBiome) ? 
            BiomeAtmosphereSettings[CurrentBiome].FogDensity : 0.02f;
        FogComponent->SetFogDensity(BaseDensity * FMath::Clamp(Intensity, 0.1f, 3.0f));
    }
}

void AEnvArt_AtmosphericManager::RefreshAtmosphericSettings()
{
    SetBiomeAtmosphere(CurrentBiome);
}

void AEnvArt_AtmosphericManager::InitializeBiomeSettings()
{
    // Savanna atmosphere
    FEnvArt_BiomeAtmosphere SavannaAtmosphere;
    SavannaAtmosphere.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    SavannaAtmosphere.FogDensity = 0.01f;
    SavannaAtmosphere.FogHeightFalloff = 0.1f;
    SavannaAtmosphere.SkyLightColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SavannaAtmosphere.SkyLightIntensity = 1.2f;
    BiomeAtmosphereSettings.Add(EBiomeType::Savanna, SavannaAtmosphere);

    // Forest atmosphere
    FEnvArt_BiomeAtmosphere ForestAtmosphere;
    ForestAtmosphere.FogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);
    ForestAtmosphere.FogDensity = 0.03f;
    ForestAtmosphere.FogHeightFalloff = 0.3f;
    ForestAtmosphere.SkyLightColor = FLinearColor(0.7f, 0.9f, 0.7f, 1.0f);
    ForestAtmosphere.SkyLightIntensity = 0.8f;
    BiomeAtmosphereSettings.Add(EBiomeType::Forest, ForestAtmosphere);

    // Swamp atmosphere
    FEnvArt_BiomeAtmosphere SwampAtmosphere;
    SwampAtmosphere.FogColor = FLinearColor(0.5f, 0.6f, 0.5f, 1.0f);
    SwampAtmosphere.FogDensity = 0.05f;
    SwampAtmosphere.FogHeightFalloff = 0.4f;
    SwampAtmosphere.SkyLightColor = FLinearColor(0.6f, 0.7f, 0.6f, 1.0f);
    SwampAtmosphere.SkyLightIntensity = 0.6f;
    BiomeAtmosphereSettings.Add(EBiomeType::Swamp, SwampAtmosphere);

    // Desert atmosphere
    FEnvArt_BiomeAtmosphere DesertAtmosphere;
    DesertAtmosphere.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    DesertAtmosphere.FogDensity = 0.008f;
    DesertAtmosphere.FogHeightFalloff = 0.05f;
    DesertAtmosphere.SkyLightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertAtmosphere.SkyLightIntensity = 1.5f;
    BiomeAtmosphereSettings.Add(EBiomeType::Desert, DesertAtmosphere);

    // Mountain atmosphere
    FEnvArt_BiomeAtmosphere MountainAtmosphere;
    MountainAtmosphere.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MountainAtmosphere.FogDensity = 0.02f;
    MountainAtmosphere.FogHeightFalloff = 0.15f;
    MountainAtmosphere.SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MountainAtmosphere.SkyLightIntensity = 1.0f;
    BiomeAtmosphereSettings.Add(EBiomeType::Mountain, MountainAtmosphere);
}

void AEnvArt_AtmosphericManager::UpdateFogSettings(const FEnvArt_BiomeAtmosphere& Settings)
{
    if (FogComponent)
    {
        FogComponent->SetFogInscatteringColor(Settings.FogColor);
        FogComponent->SetFogDensity(Settings.FogDensity);
        FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }
}

void AEnvArt_AtmosphericManager::UpdateSkyLightSettings(const FEnvArt_BiomeAtmosphere& Settings)
{
    if (SkyLightComponent)
    {
        SkyLightComponent->SetLightColor(Settings.SkyLightColor);
        SkyLightComponent->SetIntensity(Settings.SkyLightIntensity);
        SkyLightComponent->RecaptureSky();
    }
}

void AEnvArt_AtmosphericManager::InterpolateAtmosphere(const FEnvArt_BiomeAtmosphere& From, const FEnvArt_BiomeAtmosphere& To, float Alpha)
{
    FEnvArt_BiomeAtmosphere BlendedSettings;
    BlendedSettings.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    BlendedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    BlendedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    BlendedSettings.SkyLightColor = FMath::Lerp(From.SkyLightColor, To.SkyLightColor, Alpha);
    BlendedSettings.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    
    UpdateFogSettings(BlendedSettings);
    UpdateSkyLightSettings(BlendedSettings);
}