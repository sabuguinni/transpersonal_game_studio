#include "EnvArt_AtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light for sun
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetCastShadows(true);

    // Create sky light for ambient lighting
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);

    // Create height fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.5f, 0.6f, 0.7f, 1.0f));

    // Initialize default biome atmospheres
    InitializeBiomePresets();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial atmosphere based on current biome
    SetBiomeAtmosphere(CurrentBiome);
    UpdateSunPosition();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    TimeOfDay += (DeltaTime / DayDuration) * 24.0f;
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    // Update sun position based on time
    UpdateSunPosition();
    
    // Handle biome transitions if needed
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime * 2.0f; // 0.5 second transition
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
        }
    }
    
    // Update fog based on weather
    UpdateFogSettings();
}

void AEnvArt_AtmosphereManager::SetBiomeAtmosphere(EEnvArt_BiomeType BiomeType)
{
    if (BiomeAtmospheres.Contains(BiomeType))
    {
        const FEnvArt_BiomeAtmosphere& Atmosphere = BiomeAtmospheres[BiomeType];
        
        if (SunLight)
        {
            SunLight->SetLightColor(Atmosphere.SunColor);
            SunLight->SetIntensity(Atmosphere.SunIntensity);
            SunLight->SetWorldRotation(Atmosphere.SunRotation);
        }
        
        if (HeightFog)
        {
            HeightFog->SetFogDensity(Atmosphere.FogDensity);
            HeightFog->SetFogHeightFalloff(Atmosphere.FogHeightFalloff);
            HeightFog->SetFogInscatteringColor(Atmosphere.FogColor);
            HeightFog->SetStartDistance(Atmosphere.FogStartDistance);
        }
        
        CurrentBiome = BiomeType;
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
}

void AEnvArt_AtmosphereManager::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateFogSettings();
}

FLinearColor AEnvArt_AtmosphereManager::GetCurrentSunColor() const
{
    if (SunLight)
    {
        return SunLight->GetLightColor();
    }
    return FLinearColor::White;
}

float AEnvArt_AtmosphereManager::GetCurrentSunIntensity() const
{
    if (SunLight)
    {
        return SunLight->Intensity;
    }
    return 3.0f;
}

void AEnvArt_AtmosphereManager::InitializeBiomePresets()
{
    // Savana - bright, warm lighting
    FEnvArt_BiomeAtmosphere SavanaAtmo;
    SavanaAtmo.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    SavanaAtmo.SunIntensity = 4.0f;
    SavanaAtmo.SunRotation = FRotator(-45.0f, 30.0f, 0.0f);
    SavanaAtmo.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    SavanaAtmo.FogDensity = 0.01f;
    SavanaAtmo.FogHeightFalloff = 0.1f;
    BiomeAtmospheres.Add(EEnvArt_BiomeType::Savana, SavanaAtmo);
    
    // Floresta - filtered green lighting
    FEnvArt_BiomeAtmosphere FlorestaAtmo;
    FlorestaAtmo.SunColor = FLinearColor(0.8f, 1.0f, 0.8f, 1.0f);
    FlorestaAtmo.SunIntensity = 2.5f;
    FlorestaAtmo.SunRotation = FRotator(-60.0f, 45.0f, 0.0f);
    FlorestaAtmo.FogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);
    FlorestaAtmo.FogDensity = 0.03f;
    FlorestaAtmo.FogHeightFalloff = 0.3f;
    BiomeAtmospheres.Add(EEnvArt_BiomeType::Floresta, FlorestaAtmo);
    
    // Deserto - harsh, bright lighting
    FEnvArt_BiomeAtmosphere DesertoAtmo;
    DesertoAtmo.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DesertoAtmo.SunIntensity = 5.0f;
    DesertoAtmo.SunRotation = FRotator(-30.0f, 0.0f, 0.0f);
    DesertoAtmo.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    DesertoAtmo.FogDensity = 0.005f;
    DesertoAtmo.FogHeightFalloff = 0.05f;
    BiomeAtmospheres.Add(EEnvArt_BiomeType::Deserto, DesertoAtmo);
    
    // Pantano - dim, murky lighting
    FEnvArt_BiomeAtmosphere PantanoAtmo;
    PantanoAtmo.SunColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);
    PantanoAtmo.SunIntensity = 2.0f;
    PantanoAtmo.SunRotation = FRotator(-70.0f, 60.0f, 0.0f);
    PantanoAtmo.FogColor = FLinearColor(0.5f, 0.6f, 0.5f, 1.0f);
    PantanoAtmo.FogDensity = 0.05f;
    PantanoAtmo.FogHeightFalloff = 0.4f;
    BiomeAtmospheres.Add(EEnvArt_BiomeType::Pantano, PantanoAtmo);
    
    // Montanha - cool, crisp lighting
    FEnvArt_BiomeAtmosphere MontanhaAtmo;
    MontanhaAtmo.SunColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MontanhaAtmo.SunIntensity = 3.5f;
    MontanhaAtmo.SunRotation = FRotator(-40.0f, -30.0f, 0.0f);
    MontanhaAtmo.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    MontanhaAtmo.FogDensity = 0.015f;
    MontanhaAtmo.FogHeightFalloff = 0.15f;
    BiomeAtmospheres.Add(EEnvArt_BiomeType::Montanha, MontanhaAtmo);
}

void AEnvArt_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngle = (TimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at sunrise
    
    // Create rotation based on time
    FRotator SunRotation = FRotator(SunAngle, 30.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    
    // Adjust intensity based on sun height
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f));
    float IntensityMultiplier = FMath::Clamp(SunHeight, 0.1f, 1.0f);
    
    if (BiomeAtmospheres.Contains(CurrentBiome))
    {
        float BaseIntensity = BiomeAtmospheres[CurrentBiome].SunIntensity;
        SunLight->SetIntensity(BaseIntensity * IntensityMultiplier);
    }
}

void AEnvArt_AtmosphereManager::UpdateFogSettings()
{
    if (!HeightFog || !BiomeAtmospheres.Contains(CurrentBiome)) return;
    
    const FEnvArt_BiomeAtmosphere& BaseAtmo = BiomeAtmospheres[CurrentBiome];
    
    // Increase fog density with weather intensity
    float WeatherFogMultiplier = 1.0f + (WeatherIntensity * 2.0f);
    HeightFog->SetFogDensity(BaseAtmo.FogDensity * WeatherFogMultiplier);
    
    // Adjust fog color based on weather
    FLinearColor WeatherFogColor = FMath::Lerp(BaseAtmo.FogColor, 
        FLinearColor(0.3f, 0.3f, 0.4f, 1.0f), WeatherIntensity * 0.5f);
    HeightFog->SetFogInscatteringColor(WeatherFogColor);
}

void AEnvArt_AtmosphereManager::InterpolateBiomeTransition(EEnvArt_BiomeType FromBiome, EEnvArt_BiomeType ToBiome, float Alpha)
{
    if (!BiomeAtmospheres.Contains(FromBiome) || !BiomeAtmospheres.Contains(ToBiome)) return;
    
    const FEnvArt_BiomeAtmosphere& FromAtmo = BiomeAtmospheres[FromBiome];
    const FEnvArt_BiomeAtmosphere& ToAtmo = BiomeAtmospheres[ToBiome];
    
    // Interpolate sun properties
    if (SunLight)
    {
        FLinearColor LerpedColor = FMath::Lerp(FromAtmo.SunColor, ToAtmo.SunColor, Alpha);
        float LerpedIntensity = FMath::Lerp(FromAtmo.SunIntensity, ToAtmo.SunIntensity, Alpha);
        
        SunLight->SetLightColor(LerpedColor);
        SunLight->SetIntensity(LerpedIntensity);
    }
    
    // Interpolate fog properties
    if (HeightFog)
    {
        FLinearColor LerpedFogColor = FMath::Lerp(FromAtmo.FogColor, ToAtmo.FogColor, Alpha);
        float LerpedFogDensity = FMath::Lerp(FromAtmo.FogDensity, ToAtmo.FogDensity, Alpha);
        
        HeightFog->SetFogInscatteringColor(LerpedFogColor);
        HeightFog->SetFogDensity(LerpedFogDensity);
    }
}