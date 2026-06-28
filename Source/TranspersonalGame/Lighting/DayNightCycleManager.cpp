#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz — sufficient for smooth lighting

    InitDefaultSettings();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Calculate hours-per-second from day duration
    if (DayDurationSeconds > 0.0f)
    {
        HoursPerSecond = 24.0f / DayDurationSeconds;
    }

    // Auto-find lighting actors if not manually assigned
    AutoFindLightingActors();

    // Apply initial lighting state
    ApplyLightingForHour(CurrentHour);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        AdvanceTime(DeltaTime);
    }
}

void ADayNightCycleManager::AdvanceTime(float DeltaTime)
{
    CurrentHour += HoursPerSecond * DeltaTime * TimeScale;

    // Wrap around midnight
    if (CurrentHour >= 24.0f)
    {
        CurrentHour -= 24.0f;
    }

    ApplyLightingForHour(CurrentHour);
}

void ADayNightCycleManager::ApplyLightingForHour(float Hour)
{
    // Define key time points
    // Dawn: 5-7, Morning: 7-10, Midday: 10-14, Afternoon: 14-17, Dusk: 17-19, Night: 19-5

    FLight_TimeOfDaySettings CurrentSettings;

    if (Hour >= 5.0f && Hour < 7.0f)
    {
        // Dawn blend
        float Alpha = (Hour - 5.0f) / 2.0f;
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (Hour >= 7.0f && Hour < 10.0f)
    {
        // Morning blend
        float Alpha = (Hour - 7.0f) / 3.0f;
        CurrentSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (Hour >= 10.0f && Hour < 14.0f)
    {
        // Midday — peak sun
        CurrentSettings = MiddaySettings;
    }
    else if (Hour >= 14.0f && Hour < 17.0f)
    {
        // Afternoon blend toward dusk
        float Alpha = (Hour - 14.0f) / 3.0f;
        CurrentSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (Hour >= 17.0f && Hour < 19.0f)
    {
        // Dusk blend
        float Alpha = (Hour - 17.0f) / 2.0f;
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night
        CurrentSettings = NightSettings;
    }

    // Compute sun pitch from hour (arc across sky)
    // 6am = horizon (-5 deg), 12pm = zenith (-80 deg), 6pm = horizon (-5 deg)
    float SunArcProgress = FMath::Clamp((Hour - 6.0f) / 12.0f, 0.0f, 1.0f);
    float SunPitch = FMath::Lerp(-5.0f, -80.0f, FMath::Sin(SunArcProgress * PI));
    if (Hour < 6.0f || Hour > 18.0f)
    {
        SunPitch = -5.0f; // Below horizon at night
    }
    CurrentSettings.SunPitch = SunPitch;

    ApplySettingsToScene(CurrentSettings);
}

void ADayNightCycleManager::ApplySettingsToScene(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Settings.SunIntensity);
            SunComp->SetLightColor(Settings.SunColor.ToFColor(true));
        }

        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, CurrentRot.Yaw, CurrentRot.Roll));
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyTint = FLinearColor::LerpUsingHSV(A.SkyTint, B.SkyTint, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentHour >= 7.0f && CurrentHour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentHour >= 10.0f && CurrentHour < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentHour >= 14.0f && CurrentHour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour >= 17.0f && CurrentHour < 19.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour >= 19.0f && CurrentHour < 22.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

bool ADayNightCycleManager::IsNight() const
{
    return (CurrentHour >= 19.0f || CurrentHour < 5.0f);
}

float ADayNightCycleManager::GetNormalizedDayProgress() const
{
    return CurrentHour / 24.0f;
}

void ADayNightCycleManager::SetTimeOfDay(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    ApplyLightingForHour(CurrentHour);
}

void ADayNightCycleManager::PauseCycle()
{
    bCycleActive = false;
}

void ADayNightCycleManager::ResumeCycle()
{
    bCycleActive = true;
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            UDirectionalLightComponent* Comp = (*It)->GetComponentByClass<UDirectionalLightComponent>();
            if (Comp && Comp->bAtmosphereSunLight)
            {
                SunLight = *It;
                break;
            }
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

    // Find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}

void ADayNightCycleManager::InitDefaultSettings()
{
    // Dawn — warm pink-orange horizon
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.SkyTint = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.6f;

    // Midday — bright warm white
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.SkyTint = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.5f;

    // Dusk — deep orange-red
    DuskSettings.SunColor = FLinearColor(1.0f, 0.35f, 0.1f, 1.0f);
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunPitch = -6.0f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.SkyTint = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.5f;

    // Night — deep blue, minimal light
    NightSettings.SunColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);
    NightSettings.SunIntensity = 0.05f;
    NightSettings.SunPitch = -5.0f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    NightSettings.FogDensity = 0.04f;
    NightSettings.SkyTint = FLinearColor(0.05f, 0.1f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
}
