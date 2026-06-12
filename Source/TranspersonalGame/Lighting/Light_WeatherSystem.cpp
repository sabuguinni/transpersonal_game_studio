#include "Light_WeatherSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_WeatherSystem::ALight_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default weather presets
    InitializeWeatherPresets();
}

void ALight_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find existing lighting components in the level
    FindLightingComponents();

    // Apply initial weather settings
    ApplyWeatherSettings(WeatherPresets[CurrentWeatherType]);

    // Schedule automatic weather transitions if enabled
    if (bAutoWeatherTransition)
    {
        ScheduleNextWeatherTransition();
    }
}

void ALight_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update time of day progression
    if (bAutoTimeProgression)
    {
        UpdateTimeOfDay(DeltaTime);
    }

    // Update weather transition if active
    if (bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

void ALight_WeatherSystem::SetWeatherType(ELight_WeatherType NewWeatherType)
{
    if (NewWeatherType == CurrentWeatherType)
        return;

    CurrentWeatherType = NewWeatherType;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;

    if (WeatherPresets.Contains(NewWeatherType))
    {
        ApplyWeatherSettings(WeatherPresets[NewWeatherType]);
    }
}

void ALight_WeatherSystem::TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDurationSeconds)
{
    if (TargetWeather == CurrentWeatherType || !WeatherPresets.Contains(TargetWeather))
        return;

    TransitionTargetWeather = TargetWeather;
    TransitionDuration = FMath::Max(0.1f, TransitionDurationSeconds);
    TransitionTimer = 0.0f;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;

    // Store current settings as starting point
    CurrentSettings = WeatherPresets[CurrentWeatherType];
    TargetSettings = WeatherPresets[TargetWeather];
}

void ALight_WeatherSystem::SetTimeOfDay(float Hours)
{
    TimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (TimeOfDay < 0.0f)
        TimeOfDay += 24.0f;

    // Update sun position and lighting based on time
    if (SunLight && SunLight->GetLightComponent())
    {
        FRotator SunRotation = CalculateSunRotationForTime(TimeOfDay);
        SunLight->SetActorRotation(SunRotation);

        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CalculateSunIntensityForTime(TimeOfDay));
        LightComp->SetLightColor(CalculateSunColorForTime(TimeOfDay));
    }
}

void ALight_WeatherSystem::InitializeWeatherPresets()
{
    // Clear Sky - Bright Cretaceous day
    FLight_WeatherSettings ClearWeather;
    ClearWeather.SunIntensity = 5.0f;
    ClearWeather.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f); // Warm golden
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    ClearWeather.CloudCoverage = 0.2f;
    ClearWeather.WindStrength = 0.5f;
    WeatherPresets.Add(ELight_WeatherType::Clear, ClearWeather);

    // Overcast - Cloudy Cretaceous day
    FLight_WeatherSettings OvercastWeather;
    OvercastWeather.SunIntensity = 2.5f;
    OvercastWeather.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastWeather.FogDensity = 0.03f;
    OvercastWeather.FogColor = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);
    OvercastWeather.CloudCoverage = 0.8f;
    OvercastWeather.WindStrength = 1.2f;
    WeatherPresets.Add(ELight_WeatherType::Overcast, OvercastWeather);

    // Storm - Dramatic Cretaceous storm
    FLight_WeatherSettings StormWeather;
    StormWeather.SunIntensity = 1.0f;
    StormWeather.SunColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    StormWeather.FogDensity = 0.05f;
    StormWeather.FogColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.0f);
    StormWeather.CloudCoverage = 1.0f;
    StormWeather.WindStrength = 3.0f;
    WeatherPresets.Add(ELight_WeatherType::Storm, StormWeather);

    // Heavy Fog
    FLight_WeatherSettings FogWeather;
    FogWeather.SunIntensity = 1.5f;
    FogWeather.SunColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);
    FogWeather.FogDensity = 0.08f;
    FogWeather.FogColor = FLinearColor(0.85f, 0.9f, 0.95f, 1.0f);
    FogWeather.CloudCoverage = 0.6f;
    FogWeather.WindStrength = 0.3f;
    WeatherPresets.Add(ELight_WeatherType::Fog, FogWeather);

    // Dawn
    FLight_WeatherSettings DawnWeather;
    DawnWeather.SunIntensity = 3.0f;
    DawnWeather.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Orange dawn
    DawnWeather.FogDensity = 0.04f;
    DawnWeather.FogColor = FLinearColor(1.0f, 0.8f, 0.7f, 1.0f);
    DawnWeather.CloudCoverage = 0.3f;
    DawnWeather.WindStrength = 0.8f;
    WeatherPresets.Add(ELight_WeatherType::Dawn, DawnWeather);

    // Dusk
    FLight_WeatherSettings DuskWeather;
    DuskWeather.SunIntensity = 2.0f;
    DuskWeather.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f); // Red dusk
    DuskWeather.FogDensity = 0.03f;
    DuskWeather.FogColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
    DuskWeather.CloudCoverage = 0.4f;
    DuskWeather.WindStrength = 0.6f;
    WeatherPresets.Add(ELight_WeatherType::Dusk, DuskWeather);
}

void ALight_WeatherSystem::FindLightingComponents()
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

    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_WeatherSystem::ApplyWeatherSettings(const FLight_WeatherSettings& Settings)
{
    CurrentSettings = Settings;

    // Apply sun lighting
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }

    // Apply fog settings
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_WeatherSystem::UpdateTimeOfDay(float DeltaTime)
{
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    TimeOfDay += HoursPerSecond * DeltaTime;
    
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }

    // Update sun position based on time
    SetTimeOfDay(TimeOfDay);
}

void ALight_WeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    TransitionTimer += DeltaTime;
    TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    // Interpolate weather settings
    FLight_WeatherSettings InterpolatedSettings;
    InterpolatedSettings.SunIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, TransitionProgress);
    InterpolatedSettings.SunColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, TransitionProgress);
    InterpolatedSettings.FogDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, TransitionProgress);
    InterpolatedSettings.FogColor = FMath::Lerp(CurrentSettings.FogColor, TargetSettings.FogColor, TransitionProgress);
    InterpolatedSettings.CloudCoverage = FMath::Lerp(CurrentSettings.CloudCoverage, TargetSettings.CloudCoverage, TransitionProgress);
    InterpolatedSettings.WindStrength = FMath::Lerp(CurrentSettings.WindStrength, TargetSettings.WindStrength, TransitionProgress);

    ApplyWeatherSettings(InterpolatedSettings);

    // Check if transition is complete
    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentWeatherType = TransitionTargetWeather;
        CurrentSettings = TargetSettings;
    }
}

void ALight_WeatherSystem::ScheduleNextWeatherTransition()
{
    if (!bAutoWeatherTransition) return;

    float DelaySeconds = WeatherTransitionIntervalMinutes * 60.0f;
    
    GetWorldTimerManager().SetTimer(WeatherTransitionTimerHandle, [this]()
    {
        // Pick a random weather type different from current
        TArray<ELight_WeatherType> WeatherTypes = { ELight_WeatherType::Clear, ELight_WeatherType::Overcast, 
                                                   ELight_WeatherType::Storm, ELight_WeatherType::Fog };
        WeatherTypes.RemoveAll([this](ELight_WeatherType Type) { return Type == CurrentWeatherType; });
        
        if (WeatherTypes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
            TransitionToWeather(WeatherTypes[RandomIndex], 10.0f);
        }

        // Schedule next transition
        ScheduleNextWeatherTransition();
        
    }, DelaySeconds, false);
}

FLinearColor ALight_WeatherSystem::CalculateSunColorForTime(float TimeHours) const
{
    // Dawn: 5-7 AM - Orange/Red
    // Day: 7 AM - 5 PM - Warm White/Yellow
    // Dusk: 5-7 PM - Orange/Red
    // Night: 7 PM - 5 AM - Cool Blue (minimal)

    if (TimeHours >= 5.0f && TimeHours <= 7.0f) // Dawn
    {
        float Alpha = (TimeHours - 5.0f) / 2.0f;
        return FMath::Lerp(FLinearColor(1.0f, 0.4f, 0.2f, 1.0f), FLinearColor(1.0f, 0.94f, 0.78f, 1.0f), Alpha);
    }
    else if (TimeHours > 7.0f && TimeHours < 17.0f) // Day
    {
        return FLinearColor(1.0f, 0.94f, 0.78f, 1.0f); // Warm Cretaceous sunlight
    }
    else if (TimeHours >= 17.0f && TimeHours <= 19.0f) // Dusk
    {
        float Alpha = (TimeHours - 17.0f) / 2.0f;
        return FMath::Lerp(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f), FLinearColor(1.0f, 0.3f, 0.1f, 1.0f), Alpha);
    }
    else // Night
    {
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Cool moonlight
    }
}

float ALight_WeatherSystem::CalculateSunIntensityForTime(float TimeHours) const
{
    // Peak intensity at noon, minimal at night
    if (TimeHours >= 6.0f && TimeHours <= 18.0f) // Day
    {
        float NoonDistance = FMath::Abs(TimeHours - 12.0f);
        return FMath::Lerp(5.0f, 1.0f, NoonDistance / 6.0f);
    }
    else // Night
    {
        return 0.1f; // Minimal moonlight
    }
}

FRotator ALight_WeatherSystem::CalculateSunRotationForTime(float TimeHours) const
{
    // Sun moves from east to west
    // 6 AM = East (90 degrees), 12 PM = Overhead (0 degrees), 6 PM = West (-90 degrees)
    float SunAngle = (TimeHours - 12.0f) * 15.0f; // 15 degrees per hour
    return FRotator(-45.0f, SunAngle, 0.0f); // Pitch for elevation, Yaw for direction
}