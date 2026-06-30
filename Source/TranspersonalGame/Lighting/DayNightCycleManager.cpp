#include "DayNightCycleManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    InitializeDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    // Apply initial settings immediately
    ApplySettingsToWorld(MiddaySettings);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time (TimeScale = 60 means 1 real second = 1 game minute)
    const float RealSecondsPerGameDay = 86400.0f / TimeScale;
    TimeOfDayNormalized += DeltaTime / RealSecondsPerGameDay;

    // Wrap around
    if (TimeOfDayNormalized >= 1.0f)
    {
        TimeOfDayNormalized -= 1.0f;
    }

    CurrentTimeOfDay = GetTimeOfDayEnum(TimeOfDayNormalized);
    UpdateLighting(DeltaTime);
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = GetTimeOfDayEnum(TimeOfDayNormalized);
    UpdateLighting(0.0f);
}

float ADayNightCycleManager::GetAmbientTemperature() const
{
    return CurrentSettings.AmbientTemperature;
}

FVector ADayNightCycleManager::GetSunDirection() const
{
    if (SunLight)
    {
        return SunLight->GetActorForwardVector();
    }
    return FVector(-1.0f, 0.0f, -0.5f);
}

bool ADayNightCycleManager::IsDaytime() const
{
    // Daytime: 0.2 (dawn ~5am) to 0.8 (dusk ~7pm)
    return TimeOfDayNormalized >= 0.2f && TimeOfDayNormalized <= 0.8f;
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Map normalized time to lighting presets with smooth interpolation
    // 0.0 = midnight, 0.2 = dawn, 0.5 = midday, 0.75 = dusk, 1.0 = midnight

    FLight_TimeOfDaySettings TargetSettings;

    if (TimeOfDayNormalized < 0.2f)
    {
        // Midnight to Dawn
        float alpha = TimeOfDayNormalized / 0.2f;
        TargetSettings = InterpolateSettings(NightSettings, DawnSettings, alpha);
    }
    else if (TimeOfDayNormalized < 0.5f)
    {
        // Dawn to Midday
        float alpha = (TimeOfDayNormalized - 0.2f) / 0.3f;
        TargetSettings = InterpolateSettings(DawnSettings, MiddaySettings, alpha);
    }
    else if (TimeOfDayNormalized < 0.75f)
    {
        // Midday to Dusk
        float alpha = (TimeOfDayNormalized - 0.5f) / 0.25f;
        TargetSettings = InterpolateSettings(MiddaySettings, DuskSettings, alpha);
    }
    else
    {
        // Dusk to Night/Midnight
        float alpha = (TimeOfDayNormalized - 0.75f) / 0.25f;
        TargetSettings = InterpolateSettings(DuskSettings, NightSettings, alpha);
    }

    // Smooth interpolation of current settings toward target
    const float InterpSpeed = 2.0f;
    CurrentSettings.SunPitchDegrees = FMath::FInterpTo(CurrentSettings.SunPitchDegrees, TargetSettings.SunPitchDegrees, DeltaTime, InterpSpeed);
    CurrentSettings.SunIntensity = FMath::FInterpTo(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, DeltaTime, InterpSpeed);
    CurrentSettings.FogDensity = FMath::FInterpTo(CurrentSettings.FogDensity, TargetSettings.FogDensity, DeltaTime, InterpSpeed);
    CurrentSettings.SkyLightIntensity = FMath::FInterpTo(CurrentSettings.SkyLightIntensity, TargetSettings.SkyLightIntensity, DeltaTime, InterpSpeed);
    CurrentSettings.AmbientTemperature = FMath::FInterpTo(CurrentSettings.AmbientTemperature, TargetSettings.AmbientTemperature, DeltaTime, InterpSpeed);
    CurrentSettings.SunColor = FLinearColor(
        FMath::FInterpTo(CurrentSettings.SunColor.R, TargetSettings.SunColor.R, DeltaTime, InterpSpeed),
        FMath::FInterpTo(CurrentSettings.SunColor.G, TargetSettings.SunColor.G, DeltaTime, InterpSpeed),
        FMath::FInterpTo(CurrentSettings.SunColor.B, TargetSettings.SunColor.B, DeltaTime, InterpSpeed),
        1.0f
    );
    CurrentSettings.FogColor = FLinearColor(
        FMath::FInterpTo(CurrentSettings.FogColor.R, TargetSettings.FogColor.R, DeltaTime, InterpSpeed),
        FMath::FInterpTo(CurrentSettings.FogColor.G, TargetSettings.FogColor.G, DeltaTime, InterpSpeed),
        FMath::FInterpTo(CurrentSettings.FogColor.B, TargetSettings.FogColor.B, DeltaTime, InterpSpeed),
        1.0f
    );

    ApplySettingsToWorld(CurrentSettings);
}

void ADayNightCycleManager::ApplySettingsToWorld(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        // CAP GUARD: pitch must never exceed -30 (prevents underground sun)
        float SafePitch = FMath::Min(Settings.SunPitchDegrees, -30.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, Settings.SunYawDegrees, 0.0f));

        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Settings.SunIntensity);
            SunComp->SetLightColor(Settings.SunColor);
        }
    }

    // Apply to exponential height fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Settings.SkyLightIntensity);
            SkyComp->SetLightColor(Settings.SkyLightColor);
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum(float NormalizedTime) const
{
    if (NormalizedTime < 0.1f || NormalizedTime >= 0.95f) return ELight_TimeOfDay::Midnight;
    if (NormalizedTime < 0.25f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.4f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.6f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.72f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.82f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.SkyLightColor = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, Alpha),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, Alpha),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, Alpha),
        1.0f
    );
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

void ADayNightCycleManager::InitializeDefaultPresets()
{
    // === DAWN (5am) ===
    DawnSettings.SunPitchDegrees = -8.0f;   // Sun just cresting horizon
    DawnSettings.SunYawDegrees = -90.0f;     // East
    DawnSettings.SunIntensity = 2.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.55f, 0.2f, 1.0f);  // Orange-gold
    DawnSettings.FogDensity = 0.045f;
    DawnSettings.FogColor = FLinearColor(0.95f, 0.65f, 0.45f, 1.0f); // Warm orange mist
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.SkyLightColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.AmbientTemperature = 18.0f;

    // === MIDDAY (12pm) ===
    MiddaySettings.SunPitchDegrees = -75.0f;  // Nearly overhead
    MiddaySettings.SunYawDegrees = 45.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f); // Bright white-yellow
    MiddaySettings.FogDensity = 0.012f;
    MiddaySettings.FogColor = FLinearColor(0.72f, 0.82f, 0.95f, 1.0f); // Pale blue haze
    MiddaySettings.SkyLightIntensity = 2.5f;
    MiddaySettings.SkyLightColor = FLinearColor(0.85f, 0.92f, 1.0f, 1.0f);
    MiddaySettings.AmbientTemperature = 38.0f; // Brutal Cretaceous heat

    // === DUSK (7pm) ===
    DuskSettings.SunPitchDegrees = -5.0f;    // Sun at horizon
    DuskSettings.SunYawDegrees = 135.0f;     // West
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);  // Deep orange-red
    DuskSettings.FogDensity = 0.055f;
    DuskSettings.FogColor = FLinearColor(0.85f, 0.45f, 0.25f, 1.0f); // Purple-orange
    DuskSettings.SkyLightIntensity = 1.2f;
    DuskSettings.SkyLightColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DuskSettings.AmbientTemperature = 28.0f;

    // === NIGHT (midnight) ===
    NightSettings.SunPitchDegrees = -30.0f;  // CAP minimum — sun below horizon
    NightSettings.SunYawDegrees = 180.0f;
    NightSettings.SunIntensity = 0.05f;      // Moonlight only
    NightSettings.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);  // Cold blue moonlight
    NightSettings.FogDensity = 0.08f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f); // Deep blue-black
    NightSettings.SkyLightIntensity = 0.15f;
    NightSettings.SkyLightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    NightSettings.AmbientTemperature = 12.0f;

    // Initialize current settings to midday
    CurrentSettings = MiddaySettings;
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find sun if not manually assigned
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Auto-find fog if not manually assigned
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    // Auto-find sky light if not manually assigned
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: Found Sun=%s, Fog=%s, SkyLight=%s"),
        SunLight ? *SunLight->GetName() : TEXT("NONE"),
        HeightFog ? *HeightFog->GetName() : TEXT("NONE"),
        SkyLightActor ? *SkyLightActor->GetName() : TEXT("NONE"));
}
