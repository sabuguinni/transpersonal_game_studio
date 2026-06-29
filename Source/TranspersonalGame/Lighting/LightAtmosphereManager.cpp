#include "LightAtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"

ALightAtmosphereManager::ALightAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — sufficient for smooth lighting transitions

    // ─── Midday Palette (Cycle 004 — Cretaceous harsh sun) ───────────────────
    MiddayPalette.SunColor               = FLinearColor(1.0f, 0.95f, 0.75f, 1.0f);
    MiddayPalette.SunIntensity           = 12.0f;
    MiddayPalette.SunPitchAngle          = -80.0f; // near overhead
    MiddayPalette.SkyFillColor           = FLinearColor(0.4f, 0.65f, 1.0f, 1.0f);
    MiddayPalette.GroundBounceColor      = FLinearColor(0.9f, 0.55f, 0.2f, 1.0f);
    MiddayPalette.FogInscatteringColor   = FLinearColor(0.55f, 0.72f, 0.95f, 1.0f);
    MiddayPalette.FogDensity             = 0.02f;
    MiddayPalette.FogHeightFalloff       = 0.3f;
    MiddayPalette.bVolumetricFog         = true;
    MiddayPalette.VolumetricFogExtinctionScale = 0.6f;

    // ─── Golden Hour Palette ──────────────────────────────────────────────────
    GoldenHourPalette.SunColor               = FLinearColor(1.0f, 0.55f, 0.1f, 1.0f);
    GoldenHourPalette.SunIntensity           = 6.0f;
    GoldenHourPalette.SunPitchAngle          = -15.0f;
    GoldenHourPalette.SkyFillColor           = FLinearColor(0.8f, 0.35f, 0.6f, 1.0f);
    GoldenHourPalette.GroundBounceColor      = FLinearColor(0.95f, 0.4f, 0.1f, 1.0f);
    GoldenHourPalette.FogInscatteringColor   = FLinearColor(0.9f, 0.55f, 0.25f, 1.0f);
    GoldenHourPalette.FogDensity             = 0.055f;
    GoldenHourPalette.FogHeightFalloff       = 0.18f;
    GoldenHourPalette.bVolumetricFog         = true;
    GoldenHourPalette.VolumetricFogExtinctionScale = 1.2f;

    // ─── Dawn Palette ─────────────────────────────────────────────────────────
    DawnPalette.SunColor               = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DawnPalette.SunIntensity           = 3.0f;
    DawnPalette.SunPitchAngle          = -8.0f;
    DawnPalette.SkyFillColor           = FLinearColor(0.5f, 0.6f, 0.9f, 1.0f);
    DawnPalette.GroundBounceColor      = FLinearColor(0.6f, 0.45f, 0.35f, 1.0f);
    DawnPalette.FogInscatteringColor   = FLinearColor(0.7f, 0.6f, 0.8f, 1.0f);
    DawnPalette.FogDensity             = 0.08f;
    DawnPalette.FogHeightFalloff       = 0.15f;
    DawnPalette.bVolumetricFog         = true;
    DawnPalette.VolumetricFogExtinctionScale = 1.5f;

    // ─── Night Palette ────────────────────────────────────────────────────────
    NightPalette.SunColor               = FLinearColor(0.15f, 0.2f, 0.45f, 1.0f); // moonlight
    NightPalette.SunIntensity           = 0.5f;
    NightPalette.SunPitchAngle          = -45.0f;
    NightPalette.SkyFillColor           = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightPalette.GroundBounceColor      = FLinearColor(0.02f, 0.05f, 0.1f, 1.0f);
    NightPalette.FogInscatteringColor   = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
    NightPalette.FogDensity             = 0.06f;
    NightPalette.FogHeightFalloff       = 0.2f;
    NightPalette.bVolumetricFog         = true;
    NightPalette.VolumetricFogExtinctionScale = 1.0f;
}

void ALightAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    // Apply midday palette on start
    ApplyPaletteToScene(MiddayPalette);
    UpdateTimeOfDayEnum();
}

void ALightAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayNightCycleActive)
    {
        UpdateDayNightCycle(DeltaTime);
    }

    if (bTransitioning)
    {
        TransitionElapsed += DeltaTime;
        float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);

        // Lerp between source and target palette
        FLight_AtmospherePalette Blended;
        Blended.SunColor             = FMath::Lerp(TransitionSourcePalette.SunColor, TransitionTargetPalette.SunColor, Alpha);
        Blended.SunIntensity         = FMath::Lerp(TransitionSourcePalette.SunIntensity, TransitionTargetPalette.SunIntensity, Alpha);
        Blended.SunPitchAngle        = FMath::Lerp(TransitionSourcePalette.SunPitchAngle, TransitionTargetPalette.SunPitchAngle, Alpha);
        Blended.SkyFillColor         = FMath::Lerp(TransitionSourcePalette.SkyFillColor, TransitionTargetPalette.SkyFillColor, Alpha);
        Blended.GroundBounceColor    = FMath::Lerp(TransitionSourcePalette.GroundBounceColor, TransitionTargetPalette.GroundBounceColor, Alpha);
        Blended.FogInscatteringColor = FMath::Lerp(TransitionSourcePalette.FogInscatteringColor, TransitionTargetPalette.FogInscatteringColor, Alpha);
        Blended.FogDensity           = FMath::Lerp(TransitionSourcePalette.FogDensity, TransitionTargetPalette.FogDensity, Alpha);
        Blended.FogHeightFalloff     = FMath::Lerp(TransitionSourcePalette.FogHeightFalloff, TransitionTargetPalette.FogHeightFalloff, Alpha);
        Blended.bVolumetricFog       = TransitionTargetPalette.bVolumetricFog;
        Blended.VolumetricFogExtinctionScale = FMath::Lerp(
            TransitionSourcePalette.VolumetricFogExtinctionScale,
            TransitionTargetPalette.VolumetricFogExtinctionScale, Alpha);

        ApplyPaletteToScene(Blended);

        if (Alpha >= 1.0f)
        {
            bTransitioning = false;
        }
    }
}

void ALightAtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time: DayNightCycleSpeed=60 means 60 in-game seconds per real second
    // Full cycle = 86400 in-game seconds (24h)
    const float InGameSecondsPerRealSecond = DayNightCycleSpeed;
    const float InGameSecondsPerDay = 86400.0f;
    TimeOfDayNormalized += (DeltaTime * InGameSecondsPerRealSecond) / InGameSecondsPerDay;
    if (TimeOfDayNormalized > 1.0f) TimeOfDayNormalized -= 1.0f;

    UpdateTimeOfDayEnum();
}

void ALightAtmosphereManager::UpdateTimeOfDayEnum()
{
    ELight_TimeOfDay NewTime;
    const float T = TimeOfDayNormalized;

    if      (T < 0.083f) NewTime = ELight_TimeOfDay::Midnight;
    else if (T < 0.208f) NewTime = ELight_TimeOfDay::Dawn;
    else if (T < 0.333f) NewTime = ELight_TimeOfDay::Morning;
    else if (T < 0.458f) NewTime = ELight_TimeOfDay::Midday;
    else if (T < 0.583f) NewTime = ELight_TimeOfDay::Afternoon;
    else if (T < 0.708f) NewTime = ELight_TimeOfDay::GoldenHour;
    else if (T < 0.833f) NewTime = ELight_TimeOfDay::Dusk;
    else                  NewTime = ELight_TimeOfDay::Night;

    if (NewTime != PreviousTimeOfDay)
    {
        CurrentTimeOfDay = NewTime;
        PreviousTimeOfDay = NewTime;
        OnTimeOfDayChanged(NewTime);
    }
}

void ALightAtmosphereManager::ApplyPaletteToScene(const FLight_AtmospherePalette& Palette)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponent();
        if (DLC)
        {
            DLC->SetLightColor(Palette.SunColor);
            DLC->SetIntensity(Palette.SunIntensity);
        }
        FRotator SunRot = SunLight->GetActorRotation();
        SunRot.Pitch = Palette.SunPitchAngle;
        SunLight->SetActorRotation(SunRot);
    }

    // Apply to exponential height fog
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogHeightFalloff(Palette.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(Palette.FogInscatteringColor);
            FogComp->SetVolumetricFog(Palette.bVolumetricFog);
            FogComp->SetVolumetricFogExtinctionScale(Palette.VolumetricFogExtinctionScale);
        }
    }

    // Recapture sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetLightComponent();
        if (SLC)
        {
            SLC->RecaptureSky();
        }
    }
}

void ALightAtmosphereManager::ApplyPalette(const FLight_AtmospherePalette& Palette)
{
    ApplyPaletteToScene(Palette);
}

void ALightAtmosphereManager::TransitionToPalette(const FLight_AtmospherePalette& TargetPalette, float Duration)
{
    // Capture current state as source
    TransitionSourcePalette = MiddayPalette; // Default source; ideally read current scene state
    TransitionTargetPalette = TargetPalette;
    TransitionDuration = FMath::Max(Duration, 0.1f);
    TransitionElapsed = 0.0f;
    bTransitioning = true;
}

void ALightAtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    PreviousTimeOfDay = NewTime;

    switch (NewTime)
    {
        case ELight_TimeOfDay::Midday:
            ApplyPaletteToScene(MiddayPalette);
            TimeOfDayNormalized = 0.375f;
            break;
        case ELight_TimeOfDay::GoldenHour:
            ApplyPaletteToScene(GoldenHourPalette);
            TimeOfDayNormalized = 0.625f;
            break;
        case ELight_TimeOfDay::Dawn:
            ApplyPaletteToScene(DawnPalette);
            TimeOfDayNormalized = 0.167f;
            break;
        case ELight_TimeOfDay::Night:
        case ELight_TimeOfDay::Midnight:
            ApplyPaletteToScene(NightPalette);
            TimeOfDayNormalized = 0.875f;
            break;
        default:
            break;
    }

    OnTimeOfDayChanged(NewTime);
}

void ALightAtmosphereManager::SetWeather(ELight_WeatherState NewWeather)
{
    PreviousWeather = CurrentWeather;
    CurrentWeather = NewWeather;

    // Adjust fog density based on weather
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
        if (FogComp)
        {
            float WeatherFogMultiplier = 1.0f;
            switch (NewWeather)
            {
                case ELight_WeatherState::Clear:        WeatherFogMultiplier = 1.0f;  break;
                case ELight_WeatherState::PartlyCloudy: WeatherFogMultiplier = 1.3f;  break;
                case ELight_WeatherState::Overcast:     WeatherFogMultiplier = 1.8f;  break;
                case ELight_WeatherState::Rainy:        WeatherFogMultiplier = 2.5f;  break;
                case ELight_WeatherState::Stormy:       WeatherFogMultiplier = 3.5f;  break;
                case ELight_WeatherState::Foggy:        WeatherFogMultiplier = 5.0f;  break;
            }
            // Apply multiplier to current fog density
            FogComp->SetFogDensity(FogComp->FogDensity * WeatherFogMultiplier);
        }
    }

    OnWeatherChanged(NewWeather);
}

bool ALightAtmosphereManager::IsDaytime() const
{
    return TimeOfDayNormalized >= 0.25f && TimeOfDayNormalized <= 0.75f;
}

bool ALightAtmosphereManager::IsNighttime() const
{
    return !IsDaytime();
}
