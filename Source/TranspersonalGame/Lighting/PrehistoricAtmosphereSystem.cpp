#include "PrehistoricAtmosphereSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================
APrehistoricAtmosphereSystem::APrehistoricAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick 10x/sec — enough for smooth day cycle

    // Default phase settings — Dawn
    DawnSettings.SunPitchDegrees    = -5.0f;
    DawnSettings.SunIntensity       = 2.0f;
    DawnSettings.SunColor           = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity  = 0.8f;
    DawnSettings.FogDensity         = 0.08f;
    DawnSettings.FogColor           = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    DawnSettings.BloomIntensity     = 1.2f;
    DawnSettings.ExposureMin        = 0.2f;
    DawnSettings.ExposureMax        = 1.5f;

    // Morning
    MorningSettings.SunPitchDegrees    = -30.0f;
    MorningSettings.SunIntensity       = 6.0f;
    MorningSettings.SunColor           = FLinearColor(1.0f, 0.85f, 0.65f, 1.0f);
    MorningSettings.SkyLightIntensity  = 1.4f;
    MorningSettings.FogDensity         = 0.05f;
    MorningSettings.FogColor           = FLinearColor(0.65f, 0.85f, 0.55f, 1.0f);
    MorningSettings.BloomIntensity     = 0.9f;
    MorningSettings.ExposureMin        = 0.4f;
    MorningSettings.ExposureMax        = 2.5f;

    // Midday
    MiddaySettings.SunPitchDegrees    = -75.0f;
    MiddaySettings.SunIntensity       = 10.0f;
    MiddaySettings.SunColor           = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity  = 2.2f;
    MiddaySettings.FogDensity         = 0.02f;
    MiddaySettings.FogColor           = FLinearColor(0.5f, 0.75f, 0.45f, 1.0f);
    MiddaySettings.BloomIntensity     = 0.5f;
    MiddaySettings.ExposureMin        = 0.8f;
    MiddaySettings.ExposureMax        = 4.0f;

    // Dusk
    DuskSettings.SunPitchDegrees    = -8.0f;
    DuskSettings.SunIntensity       = 3.5f;
    DuskSettings.SunColor           = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskSettings.SkyLightIntensity  = 0.9f;
    DuskSettings.FogDensity         = 0.07f;
    DuskSettings.FogColor           = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DuskSettings.BloomIntensity     = 1.5f;
    DuskSettings.ExposureMin        = 0.2f;
    DuskSettings.ExposureMax        = 2.0f;

    // Night
    NightSettings.SunPitchDegrees    = 20.0f; // Below horizon
    NightSettings.SunIntensity       = 0.0f;
    NightSettings.SunColor           = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.SkyLightIntensity  = 0.3f;
    NightSettings.FogDensity         = 0.12f;
    NightSettings.FogColor           = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.BloomIntensity     = 0.3f;
    NightSettings.ExposureMin        = 0.05f;
    NightSettings.ExposureMax        = 0.8f;
}

// ============================================================
// BeginPlay
// ============================================================
void APrehistoricAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverLightingActors();
    ApplyLightingForTime(CurrentTimeNormalized);
}

// ============================================================
// Tick
// ============================================================
void APrehistoricAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        AdvanceDayCycle(DeltaTime);
    }
}

// ============================================================
// AutoDiscoverLightingActors
// ============================================================
void APrehistoricAtmosphereSystem::AutoDiscoverLightingActors()
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
    if (!SceneSkyLight)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SceneSkyLight = *It;
            break;
        }
    }
}

// ============================================================
// SetTimeOfDay
// ============================================================
void APrehistoricAtmosphereSystem::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ApplyLightingForTime(CurrentTimeNormalized);
    UpdateTimeOfDayEnum(CurrentTimeNormalized);
}

// ============================================================
// SetWeather
// ============================================================
void APrehistoricAtmosphereSystem::SetWeather(ELight_WeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        TargetWeather = NewWeather;
        CurrentWeather = NewWeather;
        OnWeatherChanged(NewWeather);
    }
}

// ============================================================
// GetCurrentHour
// ============================================================
float APrehistoricAtmosphereSystem::GetCurrentHour() const
{
    return CurrentTimeNormalized * 24.0f;
}

// ============================================================
// GetTimeOfDayLabel
// ============================================================
FString APrehistoricAtmosphereSystem::GetTimeOfDayLabel() const
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

// ============================================================
// AdvanceDayCycle (private)
// ============================================================
void APrehistoricAtmosphereSystem::AdvanceDayCycle(float DeltaTime)
{
    if (DayLengthSeconds <= 0.0f) return;

    float TimeIncrement = DeltaTime / DayLengthSeconds;
    CurrentTimeNormalized = FMath::Fmod(CurrentTimeNormalized + TimeIncrement, 1.0f);

    ApplyLightingForTime(CurrentTimeNormalized);
    UpdateTimeOfDayEnum(CurrentTimeNormalized);
}

// ============================================================
// UpdateTimeOfDayEnum (private)
// ============================================================
void APrehistoricAtmosphereSystem::UpdateTimeOfDayEnum(float NormalizedTime)
{
    float Hour = NormalizedTime * 24.0f;
    ELight_TimeOfDay NewPhase;

    if      (Hour >= 5.0f  && Hour < 7.0f)  NewPhase = ELight_TimeOfDay::Dawn;
    else if (Hour >= 7.0f  && Hour < 11.0f) NewPhase = ELight_TimeOfDay::Morning;
    else if (Hour >= 11.0f && Hour < 14.0f) NewPhase = ELight_TimeOfDay::Midday;
    else if (Hour >= 14.0f && Hour < 17.0f) NewPhase = ELight_TimeOfDay::Afternoon;
    else if (Hour >= 17.0f && Hour < 20.0f) NewPhase = ELight_TimeOfDay::Dusk;
    else if (Hour >= 20.0f && Hour < 23.0f) NewPhase = ELight_TimeOfDay::Night;
    else                                     NewPhase = ELight_TimeOfDay::Midnight;

    if (NewPhase != PreviousTimeOfDay)
    {
        CurrentTimeOfDay = NewPhase;
        PreviousTimeOfDay = NewPhase;
        OnTimeOfDayChanged(NewPhase);
    }
}

// ============================================================
// ApplyLightingForTime (private)
// ============================================================
void APrehistoricAtmosphereSystem::ApplyLightingForTime(float NormalizedTime)
{
    float Hour = NormalizedTime * 24.0f;
    FLight_DayPhaseSettings TargetSettings;

    // Blend between phases based on hour
    if (Hour >= 5.0f && Hour < 7.0f)
    {
        float Alpha = (Hour - 5.0f) / 2.0f;
        TargetSettings = InterpolatePhaseSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (Hour >= 7.0f && Hour < 11.0f)
    {
        float Alpha = (Hour - 7.0f) / 4.0f;
        TargetSettings = InterpolatePhaseSettings(DawnSettings, MorningSettings, Alpha);
    }
    else if (Hour >= 11.0f && Hour < 14.0f)
    {
        float Alpha = (Hour - 11.0f) / 3.0f;
        TargetSettings = InterpolatePhaseSettings(MorningSettings, MiddaySettings, Alpha);
    }
    else if (Hour >= 14.0f && Hour < 17.0f)
    {
        float Alpha = (Hour - 14.0f) / 3.0f;
        TargetSettings = InterpolatePhaseSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (Hour >= 17.0f && Hour < 20.0f)
    {
        float Alpha = (Hour - 17.0f) / 3.0f;
        TargetSettings = InterpolatePhaseSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        TargetSettings = NightSettings;
    }

    ApplyPhaseSettings(TargetSettings);
}

// ============================================================
// InterpolatePhaseSettings (private)
// ============================================================
FLight_DayPhaseSettings APrehistoricAtmosphereSystem::InterpolatePhaseSettings(
    const FLight_DayPhaseSettings& A,
    const FLight_DayPhaseSettings& B,
    float Alpha) const
{
    FLight_DayPhaseSettings Result;
    Result.SunPitchDegrees   = FMath::Lerp(A.SunPitchDegrees,   B.SunPitchDegrees,   Alpha);
    Result.SunIntensity      = FMath::Lerp(A.SunIntensity,      B.SunIntensity,      Alpha);
    Result.SunColor          = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity        = FMath::Lerp(A.FogDensity,        B.FogDensity,        Alpha);
    Result.FogColor          = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f);
    Result.BloomIntensity    = FMath::Lerp(A.BloomIntensity,    B.BloomIntensity,    Alpha);
    Result.ExposureMin       = FMath::Lerp(A.ExposureMin,       B.ExposureMin,       Alpha);
    Result.ExposureMax       = FMath::Lerp(A.ExposureMax,       B.ExposureMax,       Alpha);
    return Result;
}

// ============================================================
// ApplyPhaseSettings (private)
// ============================================================
void APrehistoricAtmosphereSystem::ApplyPhaseSettings(const FLight_DayPhaseSettings& Settings)
{
    // Apply to DirectionalLight (sun)
    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Settings.SunPitchDegrees, CurrentRot.Yaw, CurrentRot.Roll));

        UDirectionalLightComponent* DirComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DirComp)
        {
            DirComp->SetIntensity(Settings.SunIntensity);
            DirComp->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to SkyLight
    if (SceneSkyLight)
    {
        USkyLightComponent* SkyComp = SceneSkyLight->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Apply to HeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}
