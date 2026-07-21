#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize Cretaceous period atmospheric settings
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunTemperature = 3500.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogAlbedo = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);

    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    NoonSettings.SunTemperature = 5500.0f;
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.FogAlbedo = FLinearColor(0.78f, 0.74f, 0.67f, 1.0f);

    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunTemperature = 3000.0f;
    DuskSettings.FogDensity = 0.08f;
    DuskSettings.FogAlbedo = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);

    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.SunTemperature = 4000.0f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogAlbedo = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindWorldLightingActors();
    InitializeCretaceousAtmosphere();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }

    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        FLight_AtmosphericSettings CurrentSettings = InterpolateSettings(TransitionStartSettings, TransitionTargetSettings, Alpha);
        ApplyAtmosphericSettings(CurrentSettings);

        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void ALight_AtmosphericManager::FindWorldLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
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
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_AtmosphericManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    FLight_AtmosphericSettings Settings = GetSettingsForTimeOfDay(NewTimeOfDay);
    ApplyAtmosphericSettings(Settings);
}

void ALight_AtmosphericManager::SetWeatherType(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    
    // Modify current settings based on weather
    FLight_AtmosphericSettings CurrentSettings = GetSettingsForTimeOfDay(CurrentTimeOfDay);
    
    switch (NewWeather)
    {
        case ELight_WeatherType::Cloudy:
            CurrentSettings.SunIntensity *= 0.7f;
            CurrentSettings.FogDensity *= 1.5f;
            break;
        case ELight_WeatherType::Overcast:
            CurrentSettings.SunIntensity *= 0.4f;
            CurrentSettings.FogDensity *= 2.0f;
            CurrentSettings.SunColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
            break;
        case ELight_WeatherType::Foggy:
            CurrentSettings.SunIntensity *= 0.3f;
            CurrentSettings.FogDensity *= 5.0f;
            break;
        case ELight_WeatherType::Stormy:
            CurrentSettings.SunIntensity *= 0.2f;
            CurrentSettings.FogDensity *= 3.0f;
            CurrentSettings.SunColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
            break;
        default:
            break;
    }
    
    ApplyAtmosphericSettings(CurrentSettings);
}

void ALight_AtmosphericManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetTemperature(Settings.SunTemperature);
    }

    // Apply sky atmosphere settings
    if (SkyAtmosphere && SkyAtmosphere->GetComponent())
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
        // Configure sky atmosphere properties for Cretaceous period
        // Higher oxygen content creates different atmospheric scattering
    }

    // Apply fog settings
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetVolumetricFog(Settings.bVolumetricFog);
        FogComp->SetVolumetricFogAlbedo(Settings.FogAlbedo);
    }
}

void ALight_AtmosphericManager::TransitionToTimeOfDay(ELight_TimeOfDay TargetTime, float TransitionDuration)
{
    if (bIsTransitioning) return;

    TransitionStartSettings = GetSettingsForTimeOfDay(CurrentTimeOfDay);
    TransitionTargetSettings = GetSettingsForTimeOfDay(TargetTime);
    this->TransitionDuration = TransitionDuration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
    CurrentTimeOfDay = TargetTime;
}

void ALight_AtmosphericManager::InitializeCretaceousAtmosphere()
{
    // Set up Cretaceous period atmospheric conditions
    // Higher CO2 levels, different atmospheric composition
    // Warmer global temperatures, more humid conditions
    
    SetTimeOfDay(ELight_TimeOfDay::Noon);
    SetWeatherType(ELight_WeatherType::Clear);
    
    UE_LOG(LogTemp, Log, TEXT("Cretaceous atmospheric conditions initialized"));
}

void ALight_AtmosphericManager::ForceRefreshAtmosphere()
{
    FindWorldLightingActors();
    FLight_AtmosphericSettings CurrentSettings = GetSettingsForTimeOfDay(CurrentTimeOfDay);
    ApplyAtmosphericSettings(CurrentSettings);
}

void ALight_AtmosphericManager::UpdateDayNightCycle(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;

    float CycleSpeed = 1.0f / (DayDurationMinutes * 60.0f);
    CurrentDayTime += DeltaTime * CycleSpeed;
    
    if (CurrentDayTime >= 1.0f)
    {
        CurrentDayTime -= 1.0f;
    }

    // Determine time of day based on CurrentDayTime
    ELight_TimeOfDay NewTimeOfDay;
    if (CurrentDayTime < 0.125f || CurrentDayTime >= 0.875f)
    {
        NewTimeOfDay = ELight_TimeOfDay::Night;
    }
    else if (CurrentDayTime < 0.25f)
    {
        NewTimeOfDay = ELight_TimeOfDay::Dawn;
    }
    else if (CurrentDayTime < 0.375f)
    {
        NewTimeOfDay = ELight_TimeOfDay::Morning;
    }
    else if (CurrentDayTime < 0.625f)
    {
        NewTimeOfDay = ELight_TimeOfDay::Noon;
    }
    else if (CurrentDayTime < 0.75f)
    {
        NewTimeOfDay = ELight_TimeOfDay::Afternoon;
    }
    else
    {
        NewTimeOfDay = ELight_TimeOfDay::Dusk;
    }

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTimeOfDay);
    }
}

FLight_AtmosphericSettings ALight_AtmosphericManager::GetSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            return DawnSettings;
        case ELight_TimeOfDay::Morning:
        case ELight_TimeOfDay::Noon:
        case ELight_TimeOfDay::Afternoon:
            return NoonSettings;
        case ELight_TimeOfDay::Dusk:
            return DuskSettings;
        case ELight_TimeOfDay::Night:
            return NightSettings;
        default:
            return NoonSettings;
    }
}

FLight_AtmosphericSettings ALight_AtmosphericManager::InterpolateSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha) const
{
    FLight_AtmosphericSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunTemperature = FMath::Lerp(A.SunTemperature, B.SunTemperature, Alpha);
    Result.AtmosphereScaleHeight = FMath::Lerp(A.AtmosphereScaleHeight, B.AtmosphereScaleHeight, Alpha);
    Result.AtmosphereScattering = FMath::Lerp(A.AtmosphereScattering, B.AtmosphereScattering, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogAlbedo = FMath::Lerp(A.FogAlbedo, B.FogAlbedo, Alpha);
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    
    return Result;
}