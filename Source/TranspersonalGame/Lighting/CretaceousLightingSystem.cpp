#include "CretaceousLightingSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

ACretaceousLightingSystem::ACretaceousLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz — sufficient for smooth day/night
}

void ACretaceousLightingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lighting actors in the world if not set in editor
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    if (!AtmosphericFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }

    // Apply initial lighting state
    UpdateSunPosition(0.0f);
    UpdateFogParameters();
    UpdateSkyLight();
}

void ACretaceousLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance time of day
    if (bDayCycleActive && DayCycleSpeed > 0.0f)
    {
        TimeOfDay += DeltaTime * DayCycleSpeed / 3600.0f;
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
    }

    // Smooth weather transition
    if (!FMath::IsNearlyEqual(WeatherIntensity, TargetWeatherIntensity, 0.001f))
    {
        WeatherIntensity = FMath::FInterpTo(WeatherIntensity, TargetWeatherIntensity, DeltaTime, WeatherTransitionSpeed * 30.0f);
    }

    // Update all lighting systems
    UpdateSunPosition(DeltaTime);
    UpdateFogParameters();
    UpdateSkyLight();

    // Fire transition events
    bool bCurrentlyDaytime = IsDaytime();
    bool bCurrentlyGoldenHour = IsGoldenHour();

    if (bCurrentlyDaytime != bWasDaytime)
    {
        if (bCurrentlyDaytime)
        {
            OnDaytimeBegin();
        }
        else
        {
            OnNighttimeBegin();
        }
        bWasDaytime = bCurrentlyDaytime;
    }

    if (bCurrentlyGoldenHour != bWasGoldenHour)
    {
        if (bCurrentlyGoldenHour)
        {
            bool bIsSunrise = (TimeOfDay < 12.0f);
            OnGoldenHourBegin(bIsSunrise);
        }
        bWasGoldenHour = bCurrentlyGoldenHour;
    }
}

void ACretaceousLightingSystem::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition(0.0f);
    UpdateFogParameters();
    UpdateSkyLight();
}

float ACretaceousLightingSystem::GetNormalizedTimeOfDay() const
{
    return TimeOfDay / 24.0f;
}

bool ACretaceousLightingSystem::IsDaytime() const
{
    return TimeOfDay >= 6.0f && TimeOfDay <= 20.0f;
}

bool ACretaceousLightingSystem::IsGoldenHour() const
{
    // Sunrise: 6:00-7:30, Sunset: 18:30-20:00
    return (TimeOfDay >= 6.0f && TimeOfDay <= 7.5f) || (TimeOfDay >= 18.5f && TimeOfDay <= 20.0f);
}

void ACretaceousLightingSystem::SetWeatherIntensity(float Intensity, float TransitionDuration)
{
    TargetWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    bStormActive = (TargetWeatherIntensity > 0.5f);

    if (TransitionDuration > 0.0f)
    {
        WeatherTransitionSpeed = 1.0f / TransitionDuration;
    }
}

void ACretaceousLightingSystem::UpdateSunPosition(float DeltaTime)
{
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    float SunPitch = CalculateSunPitch();
    float SunYaw = CalculateSunYaw();
    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Calculate sun intensity based on time
    float SunIntensity = 0.0f;
    FLinearColor SunColor = NightColor;

    if (IsDaytime())
    {
        // Normalize daytime to 0-1 (6am to 8pm)
        float DayProgress = (TimeOfDay - 6.0f) / 14.0f;
        float NoonFactor = FMath::Sin(DayProgress * PI);

        if (IsGoldenHour())
        {
            // Golden hour: blend between sunrise and noon colors
            float GoldenAlpha = (TimeOfDay < 12.0f)
                ? FMath::Clamp((TimeOfDay - 6.0f) / 1.5f, 0.0f, 1.0f)
                : FMath::Clamp((20.0f - TimeOfDay) / 1.5f, 0.0f, 1.0f);
            SunColor = FLinearColor::LerpUsingHSV(SunriseColor, NoonSunColor, GoldenAlpha);
            SunIntensity = FMath::Lerp(SunriseSunsetIntensity, NoonSunIntensity, GoldenAlpha);
        }
        else
        {
            SunColor = NoonSunColor;
            SunIntensity = FMath::Lerp(SunriseSunsetIntensity, NoonSunIntensity, NoonFactor);
        }

        // Weather darkens the sun
        SunIntensity *= (1.0f - WeatherIntensity * 0.7f);
        SunColor = FLinearColor::LerpUsingHSV(SunColor, FLinearColor(0.6f, 0.65f, 0.75f, 1.0f), WeatherIntensity * 0.5f);
    }
    else
    {
        // Night: dim blue moonlight
        SunIntensity = 0.3f * (1.0f - WeatherIntensity * 0.5f);
        SunColor = NightColor;
    }

    LightComp->SetIntensity(SunIntensity);
    LightComp->SetLightColor(SunColor);
}

void ACretaceousLightingSystem::UpdateFogParameters()
{
    if (!AtmosphericFog) return;

    UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    float TargetFogDensity = DaytimeFogDensity;
    FLinearColor FogColor = FLinearColor(0.45f, 0.6f, 0.85f, 1.0f);

    if (!IsDaytime())
    {
        TargetFogDensity = NightFogDensity;
        FogColor = FLinearColor(0.08f, 0.1f, 0.2f, 1.0f);
    }
    else if (IsGoldenHour())
    {
        TargetFogDensity = GoldenHourFogDensity;
        FogColor = FLinearColor(0.85f, 0.55f, 0.25f, 1.0f);
    }

    // Weather adds dense fog
    TargetFogDensity += WeatherIntensity * 0.06f;
    FogColor = FLinearColor::LerpUsingHSV(FogColor, FLinearColor(0.5f, 0.55f, 0.6f, 1.0f), WeatherIntensity * 0.6f);

    FogComp->SetFogDensity(TargetFogDensity);
    FogComp->SetFogInscatteringColor(FogColor);
}

void ACretaceousLightingSystem::UpdateSkyLight()
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (!SkyComp) return;

    float SkyIntensity = 2.0f;
    if (!IsDaytime())
    {
        SkyIntensity = 0.4f;
    }
    else if (IsGoldenHour())
    {
        SkyIntensity = 1.2f;
    }

    SkyIntensity *= (1.0f - WeatherIntensity * 0.4f);
    SkyComp->SetIntensity(SkyIntensity);
}

FLinearColor ACretaceousLightingSystem::LerpSunColor(float Alpha) const
{
    if (Alpha < 0.5f)
    {
        return FLinearColor::LerpUsingHSV(NightColor, SunriseColor, Alpha * 2.0f);
    }
    return FLinearColor::LerpUsingHSV(SunriseColor, NoonSunColor, (Alpha - 0.5f) * 2.0f);
}

float ACretaceousLightingSystem::CalculateSunPitch() const
{
    // Map time of day to sun pitch angle
    // 6am = -5 degrees (just above horizon), 12pm = -75 degrees (high noon), 6pm = -5 degrees
    if (!IsDaytime())
    {
        // Night: sun below horizon
        return 10.0f;
    }

    float DayProgress = (TimeOfDay - 6.0f) / 14.0f; // 0.0 at 6am, 1.0 at 8pm
    float PitchCurve = FMath::Sin(DayProgress * PI);
    return FMath::Lerp(-5.0f, -75.0f, PitchCurve);
}

float ACretaceousLightingSystem::CalculateSunYaw() const
{
    // Sun travels from east (yaw=90) to west (yaw=270) over the day
    float DayProgress = (TimeOfDay - 6.0f) / 14.0f;
    return FMath::Lerp(90.0f, 270.0f, FMath::Clamp(DayProgress, 0.0f, 1.0f));
}
