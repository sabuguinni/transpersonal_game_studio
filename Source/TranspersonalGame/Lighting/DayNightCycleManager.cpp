#include "DayNightCycleManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"

// ─── Static keyframe data ─────────────────────────────────────────────────────
// 7 keyframes: Dawn(0.25), Morning(0.33), Midday(0.5), Afternoon(0.65), Dusk(0.75), Night(0.85), Midnight(0.0/1.0)

static TArray<FLight_TimeOfDaySettings> BuildKeyframes()
{
    TArray<FLight_TimeOfDaySettings> KF;
    KF.SetNum(7);

    // 0 — Dawn (0.25)
    KF[0].NormalizedTime      = 0.25f;
    KF[0].SunPitchDegrees     = -8.0f;
    KF[0].SunYawDegrees       = -80.0f;
    KF[0].SunColor            = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    KF[0].SunIntensityLux     = 8000.0f;
    KF[0].SkyColor            = FLinearColor(0.6f, 0.35f, 0.55f, 1.0f);
    KF[0].FogDensity          = 0.06f;
    KF[0].FogColor            = FLinearColor(0.7f, 0.45f, 0.35f, 1.0f);

    // 1 — Morning (0.33)
    KF[1].NormalizedTime      = 0.33f;
    KF[1].SunPitchDegrees     = -30.0f;
    KF[1].SunYawDegrees       = -60.0f;
    KF[1].SunColor            = FLinearColor(1.0f, 0.85f, 0.60f, 1.0f);
    KF[1].SunIntensityLux     = 60000.0f;
    KF[1].SkyColor            = FLinearColor(0.45f, 0.65f, 1.0f, 1.0f);
    KF[1].FogDensity          = 0.03f;
    KF[1].FogColor            = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    // 2 — Midday (0.5)
    KF[2].NormalizedTime      = 0.5f;
    KF[2].SunPitchDegrees     = -75.0f;
    KF[2].SunYawDegrees       = 0.0f;
    KF[2].SunColor            = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    KF[2].SunIntensityLux     = 120000.0f;
    KF[2].SkyColor            = FLinearColor(0.3f, 0.55f, 1.0f, 1.0f);
    KF[2].FogDensity          = 0.01f;
    KF[2].FogColor            = FLinearColor(0.5f, 0.65f, 0.9f, 1.0f);

    // 3 — Afternoon (0.65)
    KF[3].NormalizedTime      = 0.65f;
    KF[3].SunPitchDegrees     = -40.0f;
    KF[3].SunYawDegrees       = 50.0f;
    KF[3].SunColor            = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    KF[3].SunIntensityLux     = 80000.0f;
    KF[3].SkyColor            = FLinearColor(0.4f, 0.60f, 1.0f, 1.0f);
    KF[3].FogDensity          = 0.015f;
    KF[3].FogColor            = FLinearColor(0.55f, 0.65f, 0.85f, 1.0f);

    // 4 — Dusk (0.75)
    KF[4].NormalizedTime      = 0.75f;
    KF[4].SunPitchDegrees     = -6.0f;
    KF[4].SunYawDegrees       = 80.0f;
    KF[4].SunColor            = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
    KF[4].SunIntensityLux     = 5000.0f;
    KF[4].SkyColor            = FLinearColor(0.55f, 0.25f, 0.45f, 1.0f);
    KF[4].FogDensity          = 0.05f;
    KF[4].FogColor            = FLinearColor(0.75f, 0.35f, 0.20f, 1.0f);

    // 5 — Night (0.85)
    KF[5].NormalizedTime      = 0.85f;
    KF[5].SunPitchDegrees     = -5.0f;   // below horizon — acts as moon
    KF[5].SunYawDegrees       = 120.0f;
    KF[5].SunColor            = FLinearColor(0.15f, 0.20f, 0.45f, 1.0f);
    KF[5].SunIntensityLux     = 1200.0f;
    KF[5].SkyColor            = FLinearColor(0.05f, 0.08f, 0.25f, 1.0f);
    KF[5].FogDensity          = 0.04f;
    KF[5].FogColor            = FLinearColor(0.08f, 0.10f, 0.30f, 1.0f);

    // 6 — Midnight (0.0 / 1.0)
    KF[6].NormalizedTime      = 0.0f;
    KF[6].SunPitchDegrees     = -5.0f;
    KF[6].SunYawDegrees       = 180.0f;
    KF[6].SunColor            = FLinearColor(0.08f, 0.10f, 0.35f, 1.0f);
    KF[6].SunIntensityLux     = 600.0f;
    KF[6].SkyColor            = FLinearColor(0.02f, 0.04f, 0.18f, 1.0f);
    KF[6].FogDensity          = 0.06f;
    KF[6].FogColor            = FLinearColor(0.04f, 0.06f, 0.22f, 1.0f);

    return KF;
}

const TArray<FLight_TimeOfDaySettings>& ADayNightCycleManager::GetTimeKeyframes()
{
    static TArray<FLight_TimeOfDaySettings> Keyframes = BuildKeyframes();
    return Keyframes;
}

// ─── Constructor ──────────────────────────────────────────────────────────────

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz — sufficient for smooth lighting
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    CurrentNormalizedTime = StartingNormalizedTime;
    FindLightActors();
    UpdateSunPosition(CurrentNormalizedTime);
    UpdateFogSettings(CurrentNormalizedTime);
    UpdateSkyLight();
    CurrentTimeOfDay = ClassifyTimeOfDay(CurrentNormalizedTime);
    PreviousTimeOfDay = CurrentTimeOfDay;
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleRunning || DayDurationSeconds <= 0.0f) return;

    const float PrevTime = CurrentNormalizedTime;
    CurrentNormalizedTime += DeltaTime / DayDurationSeconds;

    // Day rollover
    if (CurrentNormalizedTime >= 1.0f)
    {
        CurrentNormalizedTime -= 1.0f;
        CurrentDay++;
        OnNewDayStarted(CurrentDay);
    }

    // Update sun and fog
    UpdateSunPosition(CurrentNormalizedTime);
    UpdateFogSettings(CurrentNormalizedTime);

    // Classify time of day and fire event on change
    const ELight_TimeOfDay NewTOD = ClassifyTimeOfDay(CurrentNormalizedTime);
    if (NewTOD != PreviousTimeOfDay)
    {
        CurrentTimeOfDay = NewTOD;
        PreviousTimeOfDay = NewTOD;
        OnTimeOfDayChanged(NewTOD, CurrentNormalizedTime);
    }

    // Weather transition
    if (bWeatherTransitioning && WeatherTransitionDuration > 0.0f)
    {
        WeatherTransitionAlpha += DeltaTime / WeatherTransitionDuration;
        if (WeatherTransitionAlpha >= 1.0f)
        {
            WeatherTransitionAlpha = 1.0f;
            bWeatherTransitioning = false;
            CurrentWeather.WeatherState = TargetWeather;
            OnWeatherChanged(TargetWeather);
        }
    }

    ApplyWeatherMultipliers();

    // Recapture sky every 5 seconds
    static float SkyRecaptureTimer = 0.0f;
    SkyRecaptureTimer += DeltaTime;
    if (SkyRecaptureTimer >= 5.0f)
    {
        SkyRecaptureTimer = 0.0f;
        UpdateSkyLight();
    }
}

// ─── FindLightActors ──────────────────────────────────────────────────────────

void ADayNightCycleManager::FindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}

// ─── UpdateSunPosition ────────────────────────────────────────────────────────

void ADayNightCycleManager::UpdateSunPosition(float NormalizedTime)
{
    if (!SunLight) return;

    const FLight_TimeOfDaySettings Settings = InterpolateTimeSettings(NormalizedTime);

    // Enforce pitch guard: sun must be ≤ -30° during day, allow low angles at dawn/dusk
    float FinalPitch = Settings.SunPitchDegrees;
    if (NormalizedTime > 0.3f && NormalizedTime < 0.7f)
    {
        FinalPitch = FMath::Min(FinalPitch, -30.0f);
    }

    SunLight->SetActorRotation(FRotator(FinalPitch, Settings.SunYawDegrees, 0.0f));

    UDirectionalLightComponent* DirComp = SunLight->GetComponent();
    if (DirComp)
    {
        DirComp->SetLightColor(Settings.SunColor);
        DirComp->SetIntensity(Settings.SunIntensityLux * CurrentWeather.LightIntensityMultiplier);
    }
}

// ─── UpdateFogSettings ────────────────────────────────────────────────────────

void ADayNightCycleManager::UpdateFogSettings(float NormalizedTime)
{
    if (!HeightFog) return;

    const FLight_TimeOfDaySettings Settings = InterpolateTimeSettings(NormalizedTime);

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(Settings.FogDensity * CurrentWeather.FogMultiplier);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

// ─── UpdateSkyLight ───────────────────────────────────────────────────────────

void ADayNightCycleManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (SkyComp)
    {
        SkyComp->RecaptureSky();
    }
}

// ─── ApplyWeatherMultipliers ──────────────────────────────────────────────────

void ADayNightCycleManager::ApplyWeatherMultipliers()
{
    // Weather multipliers are applied in UpdateSunPosition via CurrentWeather.LightIntensityMultiplier
    // This function can be extended for particle effects, sound changes, etc.
}

// ─── ClassifyTimeOfDay ────────────────────────────────────────────────────────

ELight_TimeOfDay ADayNightCycleManager::ClassifyTimeOfDay(float NormalizedTime) const
{
    if (NormalizedTime < 0.10f || NormalizedTime >= 0.95f) return ELight_TimeOfDay::Midnight;
    if (NormalizedTime < 0.28f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.35f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.45f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.58f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.70f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.82f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

// ─── InterpolateTimeSettings ──────────────────────────────────────────────────

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateTimeSettings(float NormalizedTime) const
{
    const TArray<FLight_TimeOfDaySettings>& KF = GetTimeKeyframes();

    // Find surrounding keyframes
    int32 PrevIdx = 0;
    int32 NextIdx = 0;

    for (int32 i = 0; i < KF.Num(); i++)
    {
        if (KF[i].NormalizedTime <= NormalizedTime)
        {
            PrevIdx = i;
        }
    }

    NextIdx = (PrevIdx + 1) % KF.Num();

    const FLight_TimeOfDaySettings& A = KF[PrevIdx];
    const FLight_TimeOfDaySettings& B = KF[NextIdx];

    float Range = B.NormalizedTime - A.NormalizedTime;
    if (Range <= 0.0f) Range = 1.0f - A.NormalizedTime + B.NormalizedTime;

    float Alpha = (NormalizedTime - A.NormalizedTime) / Range;
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    FLight_TimeOfDaySettings Result;
    Result.NormalizedTime      = NormalizedTime;
    Result.SunPitchDegrees     = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees       = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunColor            = LerpColor(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensityLux     = FMath::Lerp(A.SunIntensityLux, B.SunIntensityLux, Alpha);
    Result.SkyColor            = LerpColor(A.SkyColor, B.SkyColor, Alpha);
    Result.FogDensity          = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor            = LerpColor(A.FogColor, B.FogColor, Alpha);

    return Result;
}

// ─── LerpColor ────────────────────────────────────────────────────────────────

FLinearColor ADayNightCycleManager::LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha) const
{
    return FLinearColor(
        FMath::Lerp(A.R, B.R, Alpha),
        FMath::Lerp(A.G, B.G, Alpha),
        FMath::Lerp(A.B, B.B, Alpha),
        FMath::Lerp(A.A, B.A, Alpha)
    );
}

// ─── Public API ───────────────────────────────────────────────────────────────

void ADayNightCycleManager::SetNormalizedTime(float NewTime)
{
    CurrentNormalizedTime = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateSunPosition(CurrentNormalizedTime);
    UpdateFogSettings(CurrentNormalizedTime);
    UpdateSkyLight();
    CurrentTimeOfDay = ClassifyTimeOfDay(CurrentNormalizedTime);
}

void ADayNightCycleManager::SetWeather(ELight_WeatherState NewWeather, float TransitionDuration)
{
    if (CurrentWeather.WeatherState == NewWeather) return;

    PreviousWeather = CurrentWeather.WeatherState;
    TargetWeather = NewWeather;
    WeatherTransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    WeatherTransitionAlpha = 0.0f;
    bWeatherTransitioning = true;

    // Set weather multipliers based on state
    switch (NewWeather)
    {
        case ELight_WeatherState::Clear:
            CurrentWeather.FogMultiplier = 1.0f;
            CurrentWeather.LightIntensityMultiplier = 1.0f;
            break;
        case ELight_WeatherState::Overcast:
            CurrentWeather.FogMultiplier = 2.0f;
            CurrentWeather.LightIntensityMultiplier = 0.5f;
            break;
        case ELight_WeatherState::Rainy:
            CurrentWeather.FogMultiplier = 3.5f;
            CurrentWeather.LightIntensityMultiplier = 0.3f;
            CurrentWeather.RainIntensity = 0.7f;
            break;
        case ELight_WeatherState::Stormy:
            CurrentWeather.FogMultiplier = 5.0f;
            CurrentWeather.LightIntensityMultiplier = 0.15f;
            CurrentWeather.RainIntensity = 1.0f;
            break;
        case ELight_WeatherState::Foggy:
            CurrentWeather.FogMultiplier = 8.0f;
            CurrentWeather.LightIntensityMultiplier = 0.4f;
            break;
        case ELight_WeatherState::Hazy:
            CurrentWeather.FogMultiplier = 4.0f;
            CurrentWeather.LightIntensityMultiplier = 0.7f;
            break;
    }
}

bool ADayNightCycleManager::IsNightTime() const
{
    return (CurrentNormalizedTime < 0.28f || CurrentNormalizedTime >= 0.82f);
}

float ADayNightCycleManager::GetSunIntensityMultiplier() const
{
    const FLight_TimeOfDaySettings Settings = InterpolateTimeSettings(CurrentNormalizedTime);
    return Settings.SunIntensityLux / 120000.0f; // Normalized to midday
}

FString ADayNightCycleManager::GetTimeOfDayString() const
{
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:      return TEXT("Dawn");
        case ELight_TimeOfDay::Morning:   return TEXT("Morning");
        case ELight_TimeOfDay::Midday:    return TEXT("Midday");
        case ELight_TimeOfDay::Afternoon: return TEXT("Afternoon");
        case ELight_TimeOfDay::Dusk:      return TEXT("Dusk");
        case ELight_TimeOfDay::Night:     return TEXT("Night");
        case ELight_TimeOfDay::Midnight:  return TEXT("Midnight");
        default:                          return TEXT("Unknown");
    }
}
