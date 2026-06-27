#include "DayNightCycleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"

ULight_DayNightCycleComponent::ULight_DayNightCycleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    // Default presets — Cretaceous prehistoric world palette
    // Dawn: cool blue-pink transition
    DawnSettings.SunColor      = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SunIntensity  = 3.0f;
    DawnSettings.SunPitch      = -10.0f;
    DawnSettings.FogColor      = FLinearColor(0.7f, 0.55f, 0.8f, 1.0f);
    DawnSettings.FogDensity    = 0.05f;
    DawnSettings.SkyLightIntensity = 0.6f;
    DawnSettings.AmbientColor  = FLinearColor(0.15f, 0.1f, 0.2f, 1.0f);

    // Midday: bright warm white sun
    MiddaySettings.SunColor      = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SunIntensity  = 10.0f;
    MiddaySettings.SunPitch      = -75.0f;
    MiddaySettings.FogColor      = FLinearColor(0.6f, 0.75f, 0.95f, 1.0f);
    MiddaySettings.FogDensity    = 0.01f;
    MiddaySettings.SkyLightIntensity = 1.5f;
    MiddaySettings.AmbientColor  = FLinearColor(0.2f, 0.22f, 0.28f, 1.0f);

    // Dusk: deep amber-orange
    DuskSettings.SunColor      = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.SunIntensity  = 4.0f;
    DuskSettings.SunPitch      = -8.0f;
    DuskSettings.FogColor      = FLinearColor(0.85f, 0.45f, 0.25f, 1.0f);
    DuskSettings.FogDensity    = 0.04f;
    DuskSettings.SkyLightIntensity = 0.5f;
    DuskSettings.AmbientColor  = FLinearColor(0.18f, 0.08f, 0.05f, 1.0f);

    // Night: deep blue moonlight
    NightSettings.SunColor      = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    NightSettings.SunIntensity  = 0.5f;
    NightSettings.SunPitch      = -30.0f;
    NightSettings.FogColor      = FLinearColor(0.05f, 0.07f, 0.15f, 1.0f);
    NightSettings.FogDensity    = 0.035f;
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.AmbientColor  = FLinearColor(0.02f, 0.03f, 0.08f, 1.0f);
}

void ULight_DayNightCycleComponent::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverLightingActors();
    ApplyCurrentTimeSettings();
}

void ULight_DayNightCycleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bCycleActive)
    {
        AdvanceTime(DeltaTime);
        ApplyCurrentTimeSettings();
    }
}

void ULight_DayNightCycleComponent::AdvanceTime(float DeltaTime)
{
    // TimeScale: how many in-game seconds pass per real second
    // At TimeScale=60, 1 real second = 1 game minute
    const float GameSecondsPerRealSecond = TimeScale;
    const float GameHoursPerRealSecond = GameSecondsPerRealSecond / 3600.0f;

    CurrentHour += DeltaTime * GameHoursPerRealSecond;
    if (CurrentHour >= 24.0f)
    {
        CurrentHour -= 24.0f;
    }
}

ELight_TimeOfDay ULight_DayNightCycleComponent::GetCurrentTimeOfDay() const
{
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentHour >= 7.0f && CurrentHour < 10.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentHour >= 10.0f && CurrentHour < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentHour >= 14.0f && CurrentHour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour >= 17.0f && CurrentHour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour >= 20.0f && CurrentHour < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

float ULight_DayNightCycleComponent::GetNormalizedTime() const
{
    return CurrentHour / 24.0f;
}

void ULight_DayNightCycleComponent::SetHour(float NewHour)
{
    CurrentHour = FMath::Clamp(NewHour, 0.0f, 24.0f);
    ApplyCurrentTimeSettings();
}

void ULight_DayNightCycleComponent::AutoDiscoverLightingActors()
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

void ULight_DayNightCycleComponent::ApplyCurrentTimeSettings()
{
    FLight_TimeOfDaySettings FromSettings, ToSettings;
    float Alpha = 0.0f;
    GetPhaseSettings(CurrentHour, FromSettings, ToSettings, Alpha);

    FLight_TimeOfDaySettings Blended = LerpSettings(FromSettings, ToSettings, Alpha);
    ApplySettingsToLights(Blended);
}

void ULight_DayNightCycleComponent::GetPhaseSettings(float Hour, FLight_TimeOfDaySettings& OutFrom, FLight_TimeOfDaySettings& OutTo, float& OutAlpha) const
{
    // Phase transitions:
    // Night(0-5) -> Dawn(5-7) -> Midday(7-17) -> Dusk(17-20) -> Night(20-24)

    if (Hour < 5.0f)
    {
        // Deep night
        OutFrom = NightSettings;
        OutTo = NightSettings;
        OutAlpha = 0.0f;
    }
    else if (Hour < 7.0f)
    {
        // Night -> Dawn
        OutFrom = NightSettings;
        OutTo = DawnSettings;
        OutAlpha = (Hour - 5.0f) / 2.0f;
    }
    else if (Hour < 10.0f)
    {
        // Dawn -> Midday
        OutFrom = DawnSettings;
        OutTo = MiddaySettings;
        OutAlpha = (Hour - 7.0f) / 3.0f;
    }
    else if (Hour < 14.0f)
    {
        // Full midday
        OutFrom = MiddaySettings;
        OutTo = MiddaySettings;
        OutAlpha = 0.0f;
    }
    else if (Hour < 17.0f)
    {
        // Midday -> Dusk
        OutFrom = MiddaySettings;
        OutTo = DuskSettings;
        OutAlpha = (Hour - 14.0f) / 3.0f;
    }
    else if (Hour < 20.0f)
    {
        // Dusk -> Night
        OutFrom = DuskSettings;
        OutTo = NightSettings;
        OutAlpha = (Hour - 17.0f) / 3.0f;
    }
    else
    {
        // Full night
        OutFrom = NightSettings;
        OutTo = NightSettings;
        OutAlpha = 0.0f;
    }
}

FLight_TimeOfDaySettings ULight_DayNightCycleComponent::LerpSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunColor         = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity     = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch         = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.FogColor         = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity       = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.AmbientColor     = FMath::Lerp(A.AmbientColor, B.AmbientColor, Alpha);
    return Result;
}

void ULight_DayNightCycleComponent::ApplySettingsToLights(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to DirectionalLight (sun)
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }

        // Update sun pitch for time of day
        FRotator CurrentRot = SunLight->GetActorRotation();
        float SafePitch = FMath::Clamp(Settings.SunPitch, -89.0f, -5.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, CurrentRot.Yaw, CurrentRot.Roll));
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetLightComponent();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }
}
