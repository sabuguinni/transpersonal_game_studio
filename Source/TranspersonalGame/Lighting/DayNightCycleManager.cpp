#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions
    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneReferences();
    ApplyLightingForCurrentTime();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    TimeAccumulator += DeltaTime;
    if (TimeAccumulator >= 0.1f)
    {
        // Advance time: TimeSpeedMultiplier * real seconds = in-game hours
        // At 60x speed: 60 real seconds = 1 in-game hour
        float HoursPerSecond = TimeSpeedMultiplier / 3600.0f;
        CurrentTimeHours += HoursPerSecond * TimeAccumulator;
        if (CurrentTimeHours >= 24.0f) CurrentTimeHours -= 24.0f;

        CurrentTimeOfDayEnum = GetCurrentTimeOfDay();
        ApplyLightingForCurrentTime();
        TimeAccumulator = 0.0f;
    }
}

float ADayNightCycleManager::GetNormalizedTime() const
{
    return CurrentTimeHours / 24.0f;
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeHours >= 5.0f && CurrentTimeHours < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentTimeHours >= 7.0f && CurrentTimeHours < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentTimeHours >= 10.0f && CurrentTimeHours < 13.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeHours >= 13.0f && CurrentTimeHours < 16.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeHours >= 16.0f && CurrentTimeHours < 18.0f) return ELight_TimeOfDay::GoldenHour;
    if (CurrentTimeHours >= 18.0f && CurrentTimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeHours >= 20.0f && CurrentTimeHours < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTimeHours)
{
    CurrentTimeHours = FMath::Clamp(NewTimeHours, 0.0f, 24.0f);
    CurrentTimeOfDayEnum = GetCurrentTimeOfDay();
    ApplyLightingForCurrentTime();
}

void ADayNightCycleManager::SetTimePreset(ELight_TimeOfDay TargetTime)
{
    switch (TargetTime)
    {
        case ELight_TimeOfDay::Dawn:       SetTimeOfDay(6.0f);  break;
        case ELight_TimeOfDay::Morning:    SetTimeOfDay(8.0f);  break;
        case ELight_TimeOfDay::Midday:     SetTimeOfDay(12.0f); break;
        case ELight_TimeOfDay::Afternoon:  SetTimeOfDay(14.0f); break;
        case ELight_TimeOfDay::GoldenHour: SetTimeOfDay(17.0f); break;
        case ELight_TimeOfDay::Dusk:       SetTimeOfDay(19.0f); break;
        case ELight_TimeOfDay::Night:      SetTimeOfDay(21.0f); break;
        case ELight_TimeOfDay::Midnight:   SetTimeOfDay(0.0f);  break;
        default: break;
    }
}

void ADayNightCycleManager::AutoFindSceneReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
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
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }
}

void ADayNightCycleManager::ApplyLightingForCurrentTime()
{
    UpdateSunPosition(CurrentTimeHours);
    UpdateFogSettings(CurrentTimeHours);
    UpdateSkyLight(CurrentTimeHours);
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight) return;

    UDirectionalLightComponent* LightComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    // Sun arc: rises at 6am (pitch=-5), peaks at noon (pitch=-75), sets at 18pm (pitch=-5)
    // Night: sun below horizon
    float SunAngle = 0.0f;
    float SunPitch = 0.0f;
    float SunIntensity = 0.0f;
    FLinearColor SunColor;

    if (TimeHours >= 5.0f && TimeHours <= 19.0f)
    {
        // Daytime arc
        float DayProgress = (TimeHours - 5.0f) / 14.0f; // 0 to 1 over 14 hours
        SunPitch = FMath::Sin(DayProgress * PI) * -75.0f - 5.0f; // -5 at horizon, -75 at noon
        SunPitch = FMath::Clamp(SunPitch, -80.0f, -5.0f);

        // Sun yaw: east to west
        SunAngle = -90.0f + DayProgress * 180.0f;

        // Intensity: low at dawn/dusk, high at noon
        float IntensityMult = FMath::Sin(DayProgress * PI);
        SunIntensity = FMath::Lerp(1.0f, 12.0f, IntensityMult);

        // Color: orange at dawn/dusk, white at noon
        FLinearColor DawnColor(1.0f, 0.5f, 0.2f, 1.0f);
        FLinearColor NoonColor(1.0f, 0.98f, 0.9f, 1.0f);
        FLinearColor DuskColor(1.0f, 0.4f, 0.1f, 1.0f);

        if (DayProgress < 0.5f)
            SunColor = LerpColor(DawnColor, NoonColor, DayProgress * 2.0f);
        else
            SunColor = LerpColor(NoonColor, DuskColor, (DayProgress - 0.5f) * 2.0f);
    }
    else
    {
        // Night: sun below horizon, minimal light (moon simulation)
        SunPitch = 10.0f; // Above horizon = below ground visually
        SunAngle = 180.0f;
        SunIntensity = 0.05f;
        SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Cool blue moonlight
    }

    SunLight->SetActorRotation(FRotator(SunPitch, SunAngle, 0.0f));
    LightComp->SetIntensity(SunIntensity);
    LightComp->SetLightColor(SunColor);
}

void ADayNightCycleManager::UpdateFogSettings(float TimeHours)
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    // Fog is thicker at dawn/dusk, thinner at midday
    float FogDensity = 0.02f;
    FLinearColor FogColor;

    if (TimeHours >= 5.0f && TimeHours < 8.0f)
    {
        // Dawn: thick misty fog, cool blue-pink
        FogDensity = 0.04f;
        FogColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
    }
    else if (TimeHours >= 8.0f && TimeHours < 16.0f)
    {
        // Day: light haze, warm
        FogDensity = 0.015f;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    }
    else if (TimeHours >= 16.0f && TimeHours < 20.0f)
    {
        // Golden hour / dusk: warm orange fog
        float T = (TimeHours - 16.0f) / 4.0f;
        FogDensity = FMath::Lerp(0.02f, 0.05f, T);
        FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else
    {
        // Night: deep blue fog
        FogDensity = 0.03f;
        FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    }

    FogComp->SetFogDensity(FogDensity);
    FogComp->SetFogInscatteringColor(FogColor);
}

void ADayNightCycleManager::UpdateSkyLight(float TimeHours)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (!SkyComp) return;

    // Sky brightness follows sun arc
    float SkyBrightness = 1.0f;
    if (TimeHours >= 20.0f || TimeHours < 5.0f)
    {
        SkyBrightness = 0.1f; // Night: very dim
    }
    else if (TimeHours >= 5.0f && TimeHours < 7.0f)
    {
        SkyBrightness = FMath::Lerp(0.1f, 0.8f, (TimeHours - 5.0f) / 2.0f); // Dawn ramp up
    }
    else if (TimeHours >= 18.0f && TimeHours < 20.0f)
    {
        SkyBrightness = FMath::Lerp(0.8f, 0.1f, (TimeHours - 18.0f) / 2.0f); // Dusk ramp down
    }

    SkyComp->SetIntensity(SkyBrightness);
}

FLinearColor ADayNightCycleManager::LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const
{
    return FLinearColor(
        FMath::Lerp(A.R, B.R, Alpha),
        FMath::Lerp(A.G, B.G, Alpha),
        FMath::Lerp(A.B, B.B, Alpha),
        1.0f
    );
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    TimePresets.Empty();

    // Dawn preset
    FLight_TimePreset Dawn;
    Dawn.TimeOfDay = ELight_TimeOfDay::Dawn;
    Dawn.SunPitchDegrees = -8.0f;
    Dawn.SunYawDegrees = -90.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    Dawn.SunIntensity = 2.0f;
    Dawn.FogInscatteringColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
    Dawn.FogDensity = 0.04f;
    TimePresets.Add(Dawn);

    // Golden Hour preset
    FLight_TimePreset GoldenHour;
    GoldenHour.TimeOfDay = ELight_TimeOfDay::GoldenHour;
    GoldenHour.SunPitchDegrees = -22.0f;
    GoldenHour.SunYawDegrees = -60.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.75f, 0.3f, 1.0f);
    GoldenHour.SunIntensity = 8.0f;
    GoldenHour.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    GoldenHour.FogDensity = 0.025f;
    TimePresets.Add(GoldenHour);

    // Night preset
    FLight_TimePreset Night;
    Night.TimeOfDay = ELight_TimeOfDay::Night;
    Night.SunPitchDegrees = 10.0f;
    Night.SunYawDegrees = 180.0f;
    Night.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    Night.SunIntensity = 0.05f;
    Night.FogInscatteringColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    Night.FogDensity = 0.03f;
    TimePresets.Add(Night);
}
