#include "Light_VolumetricFogManager.h"
#include "Engine/World.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"

ALight_VolumetricFogManager::ALight_VolumetricFogManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create fog component
    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("FogComponent"));
    RootComponent = FogComponent;

    // Initialize default settings
    TimeOfDayFogIntensityMultiplier = 1.0f;
    WeatherFogIntensityMultiplier = 1.0f;
    FogTransitionSpeed = 2.0f;
    bEnableCretaceousAtmosphere = true;
    CretaceousHumidityFactor = 1.5f;
    FogUpdateInterval = 0.1f;
    
    bIsTransitioning = false;
    FogTransitionTime = 0.0f;
    FogTransitionDuration = 0.0f;
    LastFogUpdateTime = 0.0f;
}

void ALight_VolumetricFogManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeFogSettings();
    
    if (bEnableCretaceousAtmosphere)
    {
        ApplyCretaceousAtmosphericSettings();
    }
    
    // Set initial fog settings for Savana biome
    SetBiomeFog(EBiomeType::Savana);
}

void ALight_VolumetricFogManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update fog transition if active
    if (bIsTransitioning)
    {
        UpdateFogTransition(DeltaTime);
    }
    
    // Periodic fog updates based on interval
    if (CurrentTime - LastFogUpdateTime >= FogUpdateInterval)
    {
        LastFogUpdateTime = CurrentTime;
        
        // Update fog based on time of day (simplified cycle)
        float TimeOfDay = FMath::Fmod(CurrentTime / 60.0f, 24.0f); // 1 minute = 1 hour
        UpdateFogForTimeOfDay(TimeOfDay);
    }
}

void ALight_VolumetricFogManager::InitializeBiomeFogSettings()
{
    // Savana - Clear grassland with light morning mist
    FLight_BiomeFogSettings SavanaFog;
    SavanaFog.FogDensity = 0.015f;
    SavanaFog.FogHeightFalloff = 0.3f;
    SavanaFog.FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    SavanaFog.VolumetricFogScatteringDistribution = 0.1f;
    SavanaFog.VolumetricFogExtinctionScale = 0.8f;
    SavanaFog.VolumetricFogDistance = 8000.0f;
    SavanaFog.VolumetricFogAlbedo = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    BiomeFogSettings.Add(EBiomeType::Savana, SavanaFog);

    // Pantano - Dense swamp mist with green tint
    FLight_BiomeFogSettings PantanoFog;
    PantanoFog.FogDensity = 0.08f;
    PantanoFog.FogHeightFalloff = 0.15f;
    PantanoFog.FogInscatteringColor = FLinearColor(0.4f, 0.7f, 0.5f, 1.0f);
    PantanoFog.VolumetricFogScatteringDistribution = 0.4f;
    PantanoFog.VolumetricFogExtinctionScale = 1.5f;
    PantanoFog.VolumetricFogDistance = 4000.0f;
    PantanoFog.VolumetricFogAlbedo = FLinearColor(0.8f, 1.0f, 0.9f, 1.0f);
    BiomeFogSettings.Add(EBiomeType::Pantano, PantanoFog);

    // Floresta - Dappled forest mist filtering through canopy
    FLight_BiomeFogSettings FlorestaFog;
    FlorestaFog.FogDensity = 0.04f;
    FlorestaFog.FogHeightFalloff = 0.25f;
    FlorestaFog.FogInscatteringColor = FLinearColor(0.5f, 0.8f, 0.6f, 1.0f);
    FlorestaFog.VolumetricFogScatteringDistribution = 0.3f;
    FlorestaFog.VolumetricFogExtinctionScale = 1.2f;
    FlorestaFog.VolumetricFogDistance = 5000.0f;
    FlorestaFog.VolumetricFogAlbedo = FLinearColor(0.9f, 1.0f, 0.95f, 1.0f);
    BiomeFogSettings.Add(EBiomeType::Floresta, FlorestaFog);

    // Deserto - Heat shimmer and dust particles
    FLight_BiomeFogSettings DesertoFog;
    DesertoFog.FogDensity = 0.01f;
    DesertoFog.FogHeightFalloff = 0.5f;
    DesertoFog.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DesertoFog.VolumetricFogScatteringDistribution = 0.05f;
    DesertoFog.VolumetricFogExtinctionScale = 0.5f;
    DesertoFog.VolumetricFogDistance = 12000.0f;
    DesertoFog.VolumetricFogAlbedo = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    BiomeFogSettings.Add(EBiomeType::Deserto, DesertoFog);

    // Montanha - Cool mountain mist with blue tint
    FLight_BiomeFogSettings MontanhaFog;
    MontanhaFog.FogDensity = 0.06f;
    MontanhaFog.FogHeightFalloff = 0.1f;
    MontanhaFog.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    MontanhaFog.VolumetricFogScatteringDistribution = 0.2f;
    MontanhaFog.VolumetricFogExtinctionScale = 1.0f;
    MontanhaFog.VolumetricFogDistance = 6000.0f;
    MontanhaFog.VolumetricFogAlbedo = FLinearColor(0.95f, 0.98f, 1.0f, 1.0f);
    BiomeFogSettings.Add(EBiomeType::Montanha, MontanhaFog);
}

void ALight_VolumetricFogManager::SetBiomeFog(EBiomeType BiomeType)
{
    if (BiomeFogSettings.Contains(BiomeType))
    {
        const FLight_BiomeFogSettings& Settings = BiomeFogSettings[BiomeType];
        TransitionToFogSettings(Settings, FogTransitionSpeed);
    }
}

void ALight_VolumetricFogManager::UpdateFogForTimeOfDay(float TimeOfDay)
{
    if (!FogComponent)
        return;

    // Dawn (5-7): Increase fog density
    // Day (7-17): Normal fog
    // Dusk (17-19): Increase fog density
    // Night (19-5): Moderate fog with cooler tint

    float FogMultiplier = 1.0f;
    
    if (TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) // Dawn
    {
        FogMultiplier = 1.5f;
    }
    else if (TimeOfDay >= 17.0f && TimeOfDay <= 19.0f) // Dusk
    {
        FogMultiplier = 1.3f;
    }
    else if (TimeOfDay >= 19.0f || TimeOfDay <= 5.0f) // Night
    {
        FogMultiplier = 1.1f;
    }

    TimeOfDayFogIntensityMultiplier = FogMultiplier;
    
    // Apply the multiplier to current settings
    float AdjustedDensity = CurrentFogSettings.FogDensity * TimeOfDayFogIntensityMultiplier * WeatherFogIntensityMultiplier;
    FogComponent->SetFogDensity(AdjustedDensity);
}

void ALight_VolumetricFogManager::UpdateFogForWeather(EWeatherType WeatherType)
{
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            WeatherFogIntensityMultiplier = 1.0f;
            break;
        case EWeatherType::Cloudy:
            WeatherFogIntensityMultiplier = 1.2f;
            break;
        case EWeatherType::Rainy:
            WeatherFogIntensityMultiplier = 2.0f;
            break;
        case EWeatherType::Stormy:
            WeatherFogIntensityMultiplier = 1.8f;
            break;
        case EWeatherType::Foggy:
            WeatherFogIntensityMultiplier = 3.0f;
            break;
        default:
            WeatherFogIntensityMultiplier = 1.0f;
            break;
    }
}

void ALight_VolumetricFogManager::ApplyCretaceousAtmosphericSettings()
{
    if (!FogComponent)
        return;

    // Cretaceous period had higher humidity and CO2 levels
    // This creates a denser, more humid atmosphere
    
    FogComponent->SetVolumetricFog(true);
    FogComponent->SetVolumetricFogScatteringDistribution(0.2f);
    FogComponent->SetVolumetricFogExtinctionScale(1.2f * CretaceousHumidityFactor);
    FogComponent->SetVolumetricFogDistance(6000.0f);
    FogComponent->SetVolumetricFogStaticLightingScatteringIntensity(1.0f);
    FogComponent->SetVolumetricFogEmissive(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
    
    // Enhanced atmospheric scattering for prehistoric feel
    FogComponent->SetFogMaxOpacity(0.8f);
    FogComponent->SetStartDistance(0.0f);
    FogComponent->SetFogCutoffDistance(0.0f);
}

void ALight_VolumetricFogManager::TransitionToFogSettings(const FLight_BiomeFogSettings& TargetSettings, float TransitionDuration)
{
    if (!FogComponent)
        return;

    TargetFogSettings = TargetSettings;
    FogTransitionDuration = TransitionDuration;
    FogTransitionTime = 0.0f;
    bIsTransitioning = true;
}

void ALight_VolumetricFogManager::UpdateFogTransition(float DeltaTime)
{
    if (!bIsTransitioning || !FogComponent)
        return;

    FogTransitionTime += DeltaTime;
    float Alpha = FMath::Clamp(FogTransitionTime / FogTransitionDuration, 0.0f, 1.0f);
    
    // Smooth transition curve
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    // Interpolate fog settings
    FLight_BiomeFogSettings InterpolatedSettings;
    InterpolatedSettings.FogDensity = FMath::Lerp(CurrentFogSettings.FogDensity, TargetFogSettings.FogDensity, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(CurrentFogSettings.FogHeightFalloff, TargetFogSettings.FogHeightFalloff, Alpha);
    InterpolatedSettings.FogInscatteringColor = FMath::Lerp(CurrentFogSettings.FogInscatteringColor, TargetFogSettings.FogInscatteringColor, Alpha);
    InterpolatedSettings.VolumetricFogScatteringDistribution = FMath::Lerp(CurrentFogSettings.VolumetricFogScatteringDistribution, TargetFogSettings.VolumetricFogScatteringDistribution, Alpha);
    InterpolatedSettings.VolumetricFogExtinctionScale = FMath::Lerp(CurrentFogSettings.VolumetricFogExtinctionScale, TargetFogSettings.VolumetricFogExtinctionScale, Alpha);
    InterpolatedSettings.VolumetricFogDistance = FMath::Lerp(CurrentFogSettings.VolumetricFogDistance, TargetFogSettings.VolumetricFogDistance, Alpha);
    InterpolatedSettings.VolumetricFogAlbedo = FMath::Lerp(CurrentFogSettings.VolumetricFogAlbedo, TargetFogSettings.VolumetricFogAlbedo, Alpha);
    
    // Apply interpolated settings
    float AdjustedDensity = InterpolatedSettings.FogDensity * TimeOfDayFogIntensityMultiplier * WeatherFogIntensityMultiplier;
    FogComponent->SetFogDensity(AdjustedDensity);
    FogComponent->SetFogHeightFalloff(InterpolatedSettings.FogHeightFalloff);
    FogComponent->SetFogInscatteringColor(InterpolatedSettings.FogInscatteringColor);
    FogComponent->SetVolumetricFogScatteringDistribution(InterpolatedSettings.VolumetricFogScatteringDistribution);
    FogComponent->SetVolumetricFogExtinctionScale(InterpolatedSettings.VolumetricFogExtinctionScale);
    FogComponent->SetVolumetricFogDistance(InterpolatedSettings.VolumetricFogDistance);
    FogComponent->SetVolumetricFogAlbedo(InterpolatedSettings.VolumetricFogAlbedo);
    
    // Update current settings
    CurrentFogSettings = InterpolatedSettings;
    
    // Check if transition is complete
    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentFogSettings = TargetFogSettings;
    }
}