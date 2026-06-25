#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
// UCretaceousAtmosphereComponent
// ============================================================

UCretaceousAtmosphereComponent::UCretaceousAtmosphereComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UCretaceousAtmosphereComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCretaceousAtmosphereComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableDynamicDayNight)
    {
        // Advance time: full 24h cycle over DayNightCycleSpeed hours per real second
        CurrentTimeHours += DeltaTime * DayNightCycleSpeed * 24.0f / 3600.0f;
        if (CurrentTimeHours >= 24.0f)
        {
            CurrentTimeHours -= 24.0f;
        }
        CurrentTimeOfDay = GetTimeOfDayFromHours(CurrentTimeHours);
    }
}

void UCretaceousAtmosphereComponent::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    // Map enum to approximate hour
    switch (NewTime)
    {
        case ELight_TimeOfDay::Dawn:        CurrentTimeHours = 5.5f;  break;
        case ELight_TimeOfDay::Morning:     CurrentTimeHours = 8.0f;  break;
        case ELight_TimeOfDay::Midday:      CurrentTimeHours = 12.0f; break;
        case ELight_TimeOfDay::Afternoon:   CurrentTimeHours = 15.0f; break;
        case ELight_TimeOfDay::GoldenHour:  CurrentTimeHours = 17.5f; break;
        case ELight_TimeOfDay::Dusk:        CurrentTimeHours = 19.0f; break;
        case ELight_TimeOfDay::Night:       CurrentTimeHours = 21.0f; break;
        case ELight_TimeOfDay::Midnight:    CurrentTimeHours = 0.0f;  break;
        default:                            CurrentTimeHours = 12.0f; break;
    }
}

void UCretaceousAtmosphereComponent::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;

    // Adjust fog density per weather
    switch (NewWeather)
    {
        case ELight_WeatherState::Clear:        FogConfig.FogDensity = 0.02f;  break;
        case ELight_WeatherState::PartlyCloudy: FogConfig.FogDensity = 0.03f;  break;
        case ELight_WeatherState::Overcast:     FogConfig.FogDensity = 0.05f;  break;
        case ELight_WeatherState::LightRain:    FogConfig.FogDensity = 0.07f;  break;
        case ELight_WeatherState::HeavyRain:    FogConfig.FogDensity = 0.12f;  break;
        case ELight_WeatherState::Thunderstorm: FogConfig.FogDensity = 0.15f;  break;
        case ELight_WeatherState::Foggy:        FogConfig.FogDensity = 0.25f;  break;
        case ELight_WeatherState::DustStorm:    FogConfig.FogDensity = 0.18f;  break;
        default:                                FogConfig.FogDensity = 0.025f; break;
    }
}

ELight_TimeOfDay UCretaceousAtmosphereComponent::GetTimeOfDayFromHours(float Hours) const
{
    if (Hours >= 5.0f  && Hours < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (Hours >= 7.0f  && Hours < 11.0f) return ELight_TimeOfDay::Morning;
    if (Hours >= 11.0f && Hours < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hours >= 14.0f && Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours >= 17.0f && Hours < 19.0f) return ELight_TimeOfDay::GoldenHour;
    if (Hours >= 19.0f && Hours < 21.0f) return ELight_TimeOfDay::Dusk;
    if (Hours >= 21.0f && Hours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

// ============================================================
// ACretaceousLightingSystem
// ============================================================

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;

    AtmosphereComponent = CreateDefaultSubobject<UCretaceousAtmosphereComponent>(TEXT("AtmosphereComponent"));
}

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoFindLightActors)
    {
        AutoFindLightActors();
    }

    // Apply initial golden hour preset
    ApplyGoldenHourPreset();
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AtmosphereComponent && AtmosphereComponent->bEnableDynamicDayNight)
    {
        UpdateSunPosition(AtmosphereComponent->CurrentTimeHours);
    }
}

void ACretaceousLightingSystem::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    if (!SunActor)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunActor = *It;
            break;
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!FogActor)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            FogActor = *It;
            break;
        }
    }

    // Find PostProcessVolume
    if (!PostProcessActor)
    {
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            PostProcessActor = *It;
            break;
        }
    }
}

void ACretaceousLightingSystem::UpdateSunPosition(float TimeHours)
{
    if (!SunActor) return;

    // Sun arc: rises at 5h (pitch -5), peaks at 12h (pitch -80), sets at 19h (pitch -5)
    float NormalizedDayTime = FMath::Clamp((TimeHours - 5.0f) / 14.0f, 0.0f, 1.0f);
    float SunPitch = FMath::Lerp(-5.0f, -80.0f, FMath::Sin(NormalizedDayTime * PI));

    FRotator NewRotation = SunActor->GetActorRotation();
    NewRotation.Pitch = SunPitch;
    SunActor->SetActorRotation(NewRotation);

    // Update sun color and intensity
    UDirectionalLightComponent* SunComp = SunActor->GetComponentByClass<UDirectionalLightComponent>();
    if (SunComp)
    {
        SunComp->SetIntensity(GetSunIntensityForTime(TimeHours));
        FLinearColor SkyColor = GetSkyColorForTime(TimeHours);
        SunComp->SetLightColor(SkyColor);
    }
}

FLinearColor ACretaceousLightingSystem::GetSkyColorForTime(float TimeHours) const
{
    // Dawn: orange-pink, Midday: white-yellow, GoldenHour: amber, Night: deep blue
    if (TimeHours < 6.0f)   return FLinearColor(0.2f, 0.1f, 0.4f, 1.0f);  // Pre-dawn purple
    if (TimeHours < 8.0f)   return FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);  // Dawn orange
    if (TimeHours < 11.0f)  return FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);  // Morning warm white
    if (TimeHours < 15.0f)  return FLinearColor(1.0f, 0.98f, 0.9f, 1.0f); // Midday cool white
    if (TimeHours < 17.0f)  return FLinearColor(1.0f, 0.9f, 0.65f, 1.0f); // Afternoon gold
    if (TimeHours < 19.0f)  return FLinearColor(1.0f, 0.65f, 0.25f, 1.0f);// Golden hour amber
    if (TimeHours < 20.0f)  return FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);  // Dusk red
    return FLinearColor(0.05f, 0.05f, 0.2f, 1.0f);                         // Night deep blue
}

float ACretaceousLightingSystem::GetSunIntensityForTime(float TimeHours) const
{
    if (TimeHours < 5.0f || TimeHours > 20.0f) return 0.0f;   // Night
    if (TimeHours < 7.0f)  return FMath::Lerp(0.0f, 4.0f, (TimeHours - 5.0f) / 2.0f);  // Dawn ramp
    if (TimeHours < 12.0f) return FMath::Lerp(4.0f, 15.0f, (TimeHours - 7.0f) / 5.0f); // Morning rise
    if (TimeHours < 15.0f) return 15.0f;                                                  // Midday peak
    if (TimeHours < 18.0f) return FMath::Lerp(15.0f, 8.0f, (TimeHours - 15.0f) / 3.0f);// Afternoon drop
    return FMath::Lerp(8.0f, 0.0f, (TimeHours - 18.0f) / 2.0f);                         // Dusk fade
}

void ACretaceousLightingSystem::ApplyLightingPreset(ELight_TimeOfDay TimeOfDay, ELight_WeatherState Weather)
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->SetTimeOfDay(TimeOfDay);
        AtmosphereComponent->SetWeatherState(Weather);
    }
    UpdateSunPosition(AtmosphereComponent ? AtmosphereComponent->CurrentTimeHours : 14.5f);
}

void ACretaceousLightingSystem::SetSunPitch(float Pitch)
{
    if (!SunActor) return;
    FRotator R = SunActor->GetActorRotation();
    R.Pitch = Pitch;
    SunActor->SetActorRotation(R);
}

void ACretaceousLightingSystem::SetFogDensity(float Density)
{
    if (!FogActor) return;
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Density);
    }
}

void ACretaceousLightingSystem::AdvanceTimeOfDay(float DeltaHours)
{
    if (AtmosphereComponent)
    {
        AtmosphereComponent->CurrentTimeHours = FMath::Fmod(AtmosphereComponent->CurrentTimeHours + DeltaHours, 24.0f);
        AtmosphereComponent->CurrentTimeOfDay = AtmosphereComponent->GetTimeOfDayFromHours(AtmosphereComponent->CurrentTimeHours);
        UpdateSunPosition(AtmosphereComponent->CurrentTimeHours);
    }
}

void ACretaceousLightingSystem::ApplyGoldenHourPreset()
{
    ApplyLightingPreset(ELight_TimeOfDay::GoldenHour, ELight_WeatherState::Clear);
}

void ACretaceousLightingSystem::ApplyMidnightPreset()
{
    ApplyLightingPreset(ELight_TimeOfDay::Midnight, ELight_WeatherState::Clear);
}

void ACretaceousLightingSystem::ApplyDawnPreset()
{
    ApplyLightingPreset(ELight_TimeOfDay::Dawn, ELight_WeatherState::Clear);
}
