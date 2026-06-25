#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec — enough for smooth day/night

    InitializeDefaultPresets();
    CurrentPreset = GoldenHourPreset;
    TargetPreset = GoldenHourPreset;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightingActorsInWorld();
    ApplyLightingPreset(GoldenHourPreset);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }

    if (bIsTransitioning)
    {
        TickWeatherTransition(DeltaTime);
    }
}

void ACretaceousLightingManager::InitializeDefaultPresets()
{
    // Golden Hour — warm amber light, low sun, dense jungle fog
    GoldenHourPreset.SunPitch = -38.0f;
    GoldenHourPreset.SunIntensity = 12.0f;
    GoldenHourPreset.SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    GoldenHourPreset.SkyLightIntensity = 2.0f;
    GoldenHourPreset.FogDensity = 0.035f;
    GoldenHourPreset.FogColor = FLinearColor(0.6f, 0.75f, 0.5f, 1.0f);

    // Midday — bright white light, high sun, lighter fog
    MiddayPreset.SunPitch = -75.0f;
    MiddayPreset.SunIntensity = 18.0f;
    MiddayPreset.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddayPreset.SkyLightIntensity = 3.5f;
    MiddayPreset.FogDensity = 0.02f;
    MiddayPreset.FogColor = FLinearColor(0.7f, 0.85f, 0.65f, 1.0f);

    // Dawn — cool blue-pink light, very low sun, heavy morning mist
    DawnPreset.SunPitch = -5.0f;
    DawnPreset.SunIntensity = 4.0f;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.75f, 0.6f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.8f;
    DawnPreset.FogDensity = 0.08f;
    DawnPreset.FogColor = FLinearColor(0.5f, 0.6f, 0.65f, 1.0f);

    // Night — moonlight blue, stars, thick jungle darkness
    NightPreset.SunPitch = 20.0f; // Sun below horizon
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightPreset.SkyLightIntensity = 0.3f;
    NightPreset.FogDensity = 0.05f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.15f, 0.25f, 1.0f);
}

void ACretaceousLightingManager::FindLightingActorsInWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;

    // Find DirectionalLight (Sun)
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(FoundActors[0]);
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
        FogActor = FoundActors[0];
    }
}

void ACretaceousLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    TargetPreset = GetPresetForTimeOfDay(NewTime);
    bIsTransitioning = true;
    CurrentBlendAlpha = 0.0f;
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;

    // Modify fog density based on weather
    FLight_TimeOfDayPreset WeatherPreset = CurrentPreset;
    switch (NewWeather)
    {
        case ELight_WeatherState::Overcast:
            WeatherPreset.FogDensity *= 2.0f;
            WeatherPreset.SunIntensity *= 0.4f;
            WeatherPreset.SkyLightIntensity *= 0.6f;
            break;
        case ELight_WeatherState::Stormy:
            WeatherPreset.FogDensity *= 3.5f;
            WeatherPreset.SunIntensity *= 0.1f;
            WeatherPreset.SkyLightIntensity *= 0.3f;
            WeatherPreset.FogColor = FLinearColor(0.3f, 0.35f, 0.4f, 1.0f);
            break;
        case ELight_WeatherState::Foggy:
            WeatherPreset.FogDensity *= 5.0f;
            WeatherPreset.SunIntensity *= 0.5f;
            break;
        case ELight_WeatherState::HeavyRain:
            WeatherPreset.FogDensity *= 4.0f;
            WeatherPreset.SunIntensity *= 0.15f;
            WeatherPreset.FogColor = FLinearColor(0.4f, 0.45f, 0.5f, 1.0f);
            break;
        default:
            break;
    }

    TargetPreset = WeatherPreset;
    bIsTransitioning = true;
    CurrentBlendAlpha = 0.0f;
}

void ACretaceousLightingManager::ApplyLightingPreset(const FLight_TimeOfDayPreset& Preset)
{
    CurrentPreset = Preset;
    ApplySunSettings(Preset);
    ApplyFogSettings(Preset);
    ApplySkyLightSettings(Preset);
}

void ACretaceousLightingManager::ApplySunSettings(const FLight_TimeOfDayPreset& Preset)
{
    if (!SunActor) return;

    SunActor->SetActorRotation(FRotator(Preset.SunPitch, 45.0f, 0.0f));

    UDirectionalLightComponent* DLComp = SunActor->GetComponentByClass<UDirectionalLightComponent>();
    if (DLComp)
    {
        DLComp->SetIntensity(Preset.SunIntensity);
        DLComp->SetLightColor(Preset.SunColor);
    }
}

void ACretaceousLightingManager::ApplyFogSettings(const FLight_TimeOfDayPreset& Preset)
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Preset.FogDensity);
        FogComp->SetFogInscatteringColor(Preset.FogColor);
    }
}

void ACretaceousLightingManager::ApplySkyLightSettings(const FLight_TimeOfDayPreset& Preset)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SLComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (SLComp)
    {
        SLComp->SetIntensity(Preset.SkyLightIntensity);
    }
}

float ACretaceousLightingManager::GetCurrentSunPitch() const
{
    if (SunActor)
    {
        return SunActor->GetActorRotation().Pitch;
    }
    return CurrentPreset.SunPitch;
}

ELight_TimeOfDay ACretaceousLightingManager::GetTimeOfDayFromNormalized(float NormalizedTime) const
{
    // 0.0 = midnight, 0.25 = dawn, 0.5 = midday, 0.75 = dusk, 1.0 = midnight
    if (NormalizedTime < 0.2f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.3f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.4f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.55f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.65f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.75f) return ELight_TimeOfDay::GoldenHour;
    if (NormalizedTime < 0.85f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDayPreset ACretaceousLightingManager::GetPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:        return DawnPreset;
        case ELight_TimeOfDay::Morning:     return DawnPreset;   // Blend between dawn/midday
        case ELight_TimeOfDay::Midday:      return MiddayPreset;
        case ELight_TimeOfDay::Afternoon:   return MiddayPreset;
        case ELight_TimeOfDay::GoldenHour:  return GoldenHourPreset;
        case ELight_TimeOfDay::Dusk:        return GoldenHourPreset;
        case ELight_TimeOfDay::Night:       return NightPreset;
        default:                            return GoldenHourPreset;
    }
}

void ACretaceousLightingManager::TickDayNightCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    CurrentTimeNormalized += DeltaTime / DayDurationSeconds;
    if (CurrentTimeNormalized > 1.0f) CurrentTimeNormalized -= 1.0f;

    ELight_TimeOfDay NewTOD = GetTimeOfDayFromNormalized(CurrentTimeNormalized);
    if (NewTOD != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTOD;
        TargetPreset = GetPresetForTimeOfDay(NewTOD);
        bIsTransitioning = true;
        CurrentBlendAlpha = 0.0f;
    }

    // Continuous sun pitch update based on normalized time
    // Sun travels from -5 (dawn) to -75 (midday) to -5 (dusk) to +20 (night)
    float SunPitch = 0.0f;
    if (CurrentTimeNormalized < 0.5f)
    {
        // Day arc: dawn to midday to dusk
        float DayProgress = CurrentTimeNormalized / 0.5f;
        SunPitch = FMath::Lerp(-5.0f, -75.0f, FMath::Sin(DayProgress * PI));
    }
    else
    {
        // Night: sun below horizon
        SunPitch = FMath::Lerp(0.0f, 20.0f, (CurrentTimeNormalized - 0.5f) / 0.5f);
    }

    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(SunPitch, 45.0f, 0.0f));
    }
}

void ACretaceousLightingManager::TickWeatherTransition(float DeltaTime)
{
    CurrentBlendAlpha += DeltaTime / WeatherTransitionDuration;
    if (CurrentBlendAlpha >= 1.0f)
    {
        CurrentBlendAlpha = 1.0f;
        bIsTransitioning = false;
    }

    // Blend between current and target preset
    FLight_TimeOfDayPreset BlendedPreset;
    BlendedPreset.SunPitch = FMath::Lerp(CurrentPreset.SunPitch, TargetPreset.SunPitch, CurrentBlendAlpha);
    BlendedPreset.SunIntensity = FMath::Lerp(CurrentPreset.SunIntensity, TargetPreset.SunIntensity, CurrentBlendAlpha);
    BlendedPreset.SunColor = FMath::Lerp(CurrentPreset.SunColor, TargetPreset.SunColor, CurrentBlendAlpha);
    BlendedPreset.SkyLightIntensity = FMath::Lerp(CurrentPreset.SkyLightIntensity, TargetPreset.SkyLightIntensity, CurrentBlendAlpha);
    BlendedPreset.FogDensity = FMath::Lerp(CurrentPreset.FogDensity, TargetPreset.FogDensity, CurrentBlendAlpha);
    BlendedPreset.FogColor = FMath::Lerp(CurrentPreset.FogColor, TargetPreset.FogColor, CurrentBlendAlpha);

    ApplyFogSettings(BlendedPreset);
    ApplySkyLightSettings(BlendedPreset);

    if (CurrentBlendAlpha >= 1.0f)
    {
        CurrentPreset = TargetPreset;
    }
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    ApplyLightingPreset(GoldenHourPreset);
}

void ACretaceousLightingManager::ApplyMiddayPreset()
{
    ApplyLightingPreset(MiddayPreset);
}

void ACretaceousLightingManager::ApplyNightPreset()
{
    ApplyLightingPreset(NightPreset);
}
