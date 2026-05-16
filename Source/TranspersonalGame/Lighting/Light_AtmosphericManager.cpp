#include "Light_AtmosphericManager.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    AtmosphericSettings = FLight_AtmosphericSettings();
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    CurrentWeather = ELight_WeatherType::Clear;
    CurrentHour = 12.0f;
    DayDurationMinutes = 30.0f;
    bEnableDayNightCycle = true;

    // Initialize weather presets
    InitializeWeatherPresets();
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();

    // Find existing lighting actors in the level
    FindLightingActors();

    // Apply initial Cretaceous atmosphere
    ApplyCretaceousAtmosphere();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update day/night cycle if enabled
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }

    // Handle weather transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentWeather = TargetWeatherType;
        }

        // Interpolate between weather settings
        InterpolateLightingSettings(StartSettings, TargetSettings, TransitionProgress);
        UpdateAtmosphericLighting();
    }
}

void ALight_AtmosphericManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;

    // Convert time of day to hour
    switch (NewTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentHour = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentHour = 9.0f;
            break;
        case ELight_TimeOfDay::Midday:
            CurrentHour = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentHour = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentHour = 18.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentHour = 22.0f;
            break;
    }

    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::SetWeatherType(ELight_WeatherType NewWeather)
{
    if (NewWeather != CurrentWeather)
    {
        TransitionToWeather(NewWeather, 5.0f);
    }
}

void ALight_AtmosphericManager::SetCurrentHour(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    
    // Update time of day enum based on hour
    if (Hour >= 5.0f && Hour < 8.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (Hour >= 8.0f && Hour < 11.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (Hour >= 11.0f && Hour < 14.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (Hour >= 14.0f && Hour < 17.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (Hour >= 17.0f && Hour < 20.0f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;

    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::ApplyCretaceousAtmosphere()
{
    ConfigureTropicalLighting();
    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (!SunLight) return;

    FRotator SunRotation = CalculateSunRotation();
    SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphericManager::UpdateAtmosphericLighting()
{
    // Update sun light
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(AtmosphericSettings.SunIntensity);
            LightComp->SetLightColor(AtmosphericSettings.SunColor);
            LightComp->SetTemperature(AtmosphericSettings.SunTemperature);
        }
    }

    // Update sky atmosphere
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
        if (SkyComp)
        {
            SkyComp->SetRayleighScatteringScale(AtmosphericSettings.RayleighScattering);
            SkyComp->SetMieScatteringScale(AtmosphericSettings.MieScattering);
        }
    }

    // Update height fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
            FogComp->SetFogInscatteringColor(AtmosphericSettings.FogColor);
        }
    }
}

void ALight_AtmosphericManager::ConfigureTropicalLighting()
{
    // Set tropical Cretaceous atmosphere settings
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AtmosphericSettings.SunTemperature = 5500.0f;
    AtmosphericSettings.RayleighScattering = 0.8f;
    AtmosphericSettings.MieScattering = 0.2f;
    AtmosphericSettings.FogDensity = 0.005f;
    AtmosphericSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
}

void ALight_AtmosphericManager::TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration)
{
    if (bIsTransitioning) return;

    TargetWeatherType = TargetWeather;
    this->TransitionDuration = TransitionDuration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;

    // Store current settings as start point
    StartSettings = AtmosphericSettings;
    TargetSettings = GetSettingsForWeather(TargetWeather);
}

void ALight_AtmosphericManager::ApplyStormLighting()
{
    AtmosphericSettings = StormWeatherSettings;
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::ApplyClearWeatherLighting()
{
    AtmosphericSettings = ClearWeatherSettings;
    UpdateAtmosphericLighting();
}

void ALight_AtmosphericManager::UpdateDayNightCycle(float DeltaTime)
{
    // Convert real time to game time
    float GameTimeIncrement = (DeltaTime / 60.0f) * (24.0f / DayDurationMinutes);
    CurrentHour += GameTimeIncrement;

    // Wrap around 24 hours
    if (CurrentHour >= 24.0f)
    {
        CurrentHour -= 24.0f;
    }

    // Update time of day and sun position
    SetCurrentHour(CurrentHour);
}

void ALight_AtmosphericManager::InterpolateLightingSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    AtmosphericSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    AtmosphericSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    AtmosphericSettings.SunTemperature = FMath::Lerp(From.SunTemperature, To.SunTemperature, Alpha);
    AtmosphericSettings.RayleighScattering = FMath::Lerp(From.RayleighScattering, To.RayleighScattering, Alpha);
    AtmosphericSettings.MieScattering = FMath::Lerp(From.MieScattering, To.MieScattering, Alpha);
    AtmosphericSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    AtmosphericSettings.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
}

float ALight_AtmosphericManager::CalculateSunAngle() const
{
    // Convert hour to angle (0-360 degrees)
    // 6 AM = 0 degrees (horizon), 12 PM = 90 degrees (zenith), 6 PM = 180 degrees (horizon)
    float NormalizedHour = (CurrentHour - 6.0f) / 12.0f; // Normalize to 0-2 range
    float SunAngle = NormalizedHour * 180.0f; // Convert to 0-360 degrees
    return FMath::Clamp(SunAngle, -90.0f, 270.0f);
}

FRotator ALight_AtmosphericManager::CalculateSunRotation() const
{
    float SunAngle = CalculateSunAngle();
    
    // Calculate sun elevation (pitch)
    float SunElevation = -90.0f + SunAngle;
    if (SunElevation > 90.0f)
    {
        SunElevation = 180.0f - SunElevation;
    }

    // Sun moves from east to west
    float SunAzimuth = 90.0f + (CurrentHour - 6.0f) * 15.0f; // 15 degrees per hour

    return FRotator(SunElevation, SunAzimuth, 0.0f);
}

void ALight_AtmosphericManager::InitializeWeatherPresets()
{
    // Clear weather (default tropical Cretaceous)
    ClearWeatherSettings.SunIntensity = 5.0f;
    ClearWeatherSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    ClearWeatherSettings.SunTemperature = 5500.0f;
    ClearWeatherSettings.RayleighScattering = 0.8f;
    ClearWeatherSettings.MieScattering = 0.2f;
    ClearWeatherSettings.FogDensity = 0.005f;
    ClearWeatherSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Cloudy weather
    CloudyWeatherSettings.SunIntensity = 3.0f;
    CloudyWeatherSettings.SunColor = FLinearColor(0.9f, 0.9f, 0.95f, 1.0f);
    CloudyWeatherSettings.SunTemperature = 6000.0f;
    CloudyWeatherSettings.RayleighScattering = 1.0f;
    CloudyWeatherSettings.MieScattering = 0.4f;
    CloudyWeatherSettings.FogDensity = 0.01f;
    CloudyWeatherSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    // Storm weather
    StormWeatherSettings.SunIntensity = 1.5f;
    StormWeatherSettings.SunColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    StormWeatherSettings.SunTemperature = 6500.0f;
    StormWeatherSettings.RayleighScattering = 1.2f;
    StormWeatherSettings.MieScattering = 0.6f;
    StormWeatherSettings.FogDensity = 0.02f;
    StormWeatherSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    // Foggy weather
    FoggyWeatherSettings.SunIntensity = 2.0f;
    FoggyWeatherSettings.SunColor = FLinearColor(0.95f, 0.95f, 0.9f, 1.0f);
    FoggyWeatherSettings.SunTemperature = 5800.0f;
    FoggyWeatherSettings.RayleighScattering = 0.6f;
    FoggyWeatherSettings.MieScattering = 0.8f;
    FoggyWeatherSettings.FogDensity = 0.05f;
    FoggyWeatherSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
}

FLight_AtmosphericSettings ALight_AtmosphericManager::GetSettingsForWeather(ELight_WeatherType Weather) const
{
    switch (Weather)
    {
        case ELight_WeatherType::Clear:
            return ClearWeatherSettings;
        case ELight_WeatherType::Cloudy:
            return CloudyWeatherSettings;
        case ELight_WeatherType::Overcast:
            return CloudyWeatherSettings; // Use cloudy settings for overcast
        case ELight_WeatherType::Foggy:
            return FoggyWeatherSettings;
        case ELight_WeatherType::Storm:
            return StormWeatherSettings;
        default:
            return ClearWeatherSettings;
    }
}