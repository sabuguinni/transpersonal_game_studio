#include "EnvArt_AtmosphereController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ConstructorHelpers.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default settings
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
    DawnSettings.FogDensity = 0.05f;

    NoonSettings.SunAngle = 75.0f;
    NoonSettings.SunColor = FLinearColor::White;
    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    NoonSettings.FogDensity = 0.01f;

    DuskSettings.SunAngle = 25.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskSettings.FogDensity = 0.03f;

    NightSettings.SunAngle = -15.0f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.FogDensity = 0.02f;

    // Initialize weather
    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.WindStrength = 1.0f;
    CurrentWeather.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    CurrentWeather.bIsRaining = false;
    CurrentWeather.RainIntensity = 0.0f;

    // Create particle components
    DustParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    PollenParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PollenParticles"));
    RainParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainParticles"));

    if (DustParticles)
    {
        DustParticles->SetAutoActivate(false);
        RootComponent = DustParticles;
    }

    if (PollenParticles)
    {
        PollenParticles->SetAutoActivate(false);
        PollenParticles->SetupAttachment(RootComponent);
    }

    if (RainParticles)
    {
        RainParticles->SetAutoActivate(false);
        RainParticles->SetupAttachment(RootComponent);
    }
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();

    // Find existing lighting actors in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find height fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        if (FogActor)
        {
            HeightFog = FogActor->GetComponent();
        }
    }

    // Initialize atmosphere
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        TimeAccumulator += DeltaTime;
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;

        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }

        UpdateAtmosphericEffects();
    }
}

void AEnvArt_AtmosphereController::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateAtmosphericEffects();
}

void AEnvArt_AtmosphereController::SetWeatherPreset(const FEnvArt_WeatherSettings& NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateParticleEffects();
}

void AEnvArt_AtmosphereController::CreateGoldenHourLighting()
{
    SetTimeOfDay(18.0f); // 6 PM golden hour
    
    FEnvArt_WeatherSettings GoldenWeather;
    GoldenWeather.CloudCoverage = 0.2f;
    GoldenWeather.WindStrength = 0.5f;
    GoldenWeather.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    GoldenWeather.bIsRaining = false;
    GoldenWeather.RainIntensity = 0.0f;
    
    SetWeatherPreset(GoldenWeather);

    // Activate dust particles for atmospheric effect
    if (DustParticles)
    {
        DustParticles->SetActive(true);
    }
}

void AEnvArt_AtmosphereController::CreateStormyWeather()
{
    FEnvArt_WeatherSettings StormWeather;
    StormWeather.CloudCoverage = 0.9f;
    StormWeather.WindStrength = 3.0f;
    StormWeather.WindDirection = FVector(1.0f, 0.5f, 0.0f);
    StormWeather.bIsRaining = true;
    StormWeather.RainIntensity = 0.8f;
    
    SetWeatherPreset(StormWeather);

    if (RainParticles)
    {
        RainParticles->SetActive(true);
    }
}

void AEnvArt_AtmosphereController::CreateClearWeather()
{
    FEnvArt_WeatherSettings ClearWeather;
    ClearWeather.CloudCoverage = 0.1f;
    ClearWeather.WindStrength = 0.3f;
    ClearWeather.WindDirection = FVector(1.0f, 0.0f, 0.0f);
    ClearWeather.bIsRaining = false;
    ClearWeather.RainIntensity = 0.0f;
    
    SetWeatherPreset(ClearWeather);

    if (PollenParticles)
    {
        PollenParticles->SetActive(true);
    }
}

void AEnvArt_AtmosphereController::UpdateAtmosphericEffects()
{
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateParticleEffects();
}

void AEnvArt_AtmosphereController::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }

    // Determine current time period and interpolate settings
    FEnvArt_TimeOfDaySettings CurrentSettings;
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 10.0f)
    {
        // Dawn to morning
        float Alpha = (CurrentTimeOfDay - 5.0f) / 5.0f;
        InterpolateTimeOfDaySettings(Alpha, DawnSettings, NoonSettings, CurrentSettings);
    }
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 16.0f)
    {
        // Morning to afternoon (noon settings)
        CurrentSettings = NoonSettings;
    }
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 20.0f)
    {
        // Afternoon to dusk
        float Alpha = (CurrentTimeOfDay - 16.0f) / 4.0f;
        InterpolateTimeOfDaySettings(Alpha, NoonSettings, DuskSettings, CurrentSettings);
    }
    else if (CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 5.0f)
    {
        // Night
        CurrentSettings = NightSettings;
    }

    // Apply sun settings
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    LightComp->SetLightColor(CurrentSettings.SunColor);
    LightComp->SetIntensity(CurrentSettings.SunIntensity);

    // Calculate sun rotation based on time
    float SunRotationPitch = CurrentSettings.SunAngle;
    float SunRotationYaw = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start from east
    
    FRotator SunRotation(SunRotationPitch, SunRotationYaw, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void AEnvArt_AtmosphereController::UpdateFogSettings()
{
    if (!HeightFog)
    {
        return;
    }

    // Get current time-based fog settings
    FEnvArt_TimeOfDaySettings CurrentSettings;
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 10.0f)
    {
        float Alpha = (CurrentTimeOfDay - 5.0f) / 5.0f;
        InterpolateTimeOfDaySettings(Alpha, DawnSettings, NoonSettings, CurrentSettings);
    }
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 16.0f)
    {
        CurrentSettings = NoonSettings;
    }
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 20.0f)
    {
        float Alpha = (CurrentTimeOfDay - 16.0f) / 4.0f;
        InterpolateTimeOfDaySettings(Alpha, NoonSettings, DuskSettings, CurrentSettings);
    }
    else
    {
        CurrentSettings = NightSettings;
    }

    // Apply fog settings with weather influence
    float WeatherDensityMultiplier = 1.0f + (CurrentWeather.CloudCoverage * 2.0f);
    float FinalFogDensity = CurrentSettings.FogDensity * WeatherDensityMultiplier;

    HeightFog->SetFogInscatteringColor(CurrentSettings.FogColor);
    HeightFog->SetFogDensity(FinalFogDensity);
    HeightFog->SetFogHeightFalloff(CurrentSettings.FogHeightFalloff);
}

void AEnvArt_AtmosphereController::UpdateParticleEffects()
{
    // Update dust particles based on wind
    if (DustParticles)
    {
        bool bShouldShowDust = CurrentWeather.WindStrength > 0.5f && !CurrentWeather.bIsRaining;
        DustParticles->SetActive(bShouldShowDust);
    }

    // Update pollen particles for clear weather
    if (PollenParticles)
    {
        bool bShouldShowPollen = CurrentWeather.CloudCoverage < 0.3f && !CurrentWeather.bIsRaining;
        PollenParticles->SetActive(bShouldShowPollen);
    }

    // Update rain particles
    if (RainParticles)
    {
        RainParticles->SetActive(CurrentWeather.bIsRaining);
    }
}

void AEnvArt_AtmosphereController::InterpolateTimeOfDaySettings(float Alpha, const FEnvArt_TimeOfDaySettings& A, const FEnvArt_TimeOfDaySettings& B, FEnvArt_TimeOfDaySettings& Result)
{
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
}