#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth cycle

    // Dawn preset - warm pink/orange
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunYaw = -90.0f;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogInscatteringColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);

    // Midday preset - bright white/yellow
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunYaw = 0.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    // Dusk preset - deep orange/red
    DuskSettings.SunPitch = -5.0f;
    DuskSettings.SunYaw = 90.0f;
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.FogInscatteringColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);

    // Night preset - dark blue
    NightSettings.SunPitch = 30.0f; // Below horizon
    NightSettings.SunYaw = 180.0f;
    NightSettings.SunIntensity = 0.0f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SkyLightIntensity = 0.1f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogInscatteringColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        UpdateLighting(DeltaTime);
    }
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance time
    // TimeSpeed = 60 means 1 real second = 1 game minute
    float HoursPerSecond = TimeSpeed / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Update sun rotation
    if (SunLight)
    {
        float Pitch = GetSunPitchForTime(CurrentTimeOfDay);
        float Yaw = GetSunYawForTime(CurrentTimeOfDay);
        SunLight->SetActorRotation(FRotator(Pitch, Yaw, 0.0f));

        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(GetSunIntensityForTime(CurrentTimeOfDay));
            SunComp->SetLightColor(GetSunColorForTime(CurrentTimeOfDay));
        }
    }

    // Update fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(GetFogDensityForTime(CurrentTimeOfDay));
            FogComp->SetFogInscatteringColor(GetFogColorForTime(CurrentTimeOfDay));
        }
    }

    // Update skylight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            float SkyIntensity = FMath::Lerp(0.1f, 2.0f, FMath::Clamp((CurrentTimeOfDay - 6.0f) / 6.0f, 0.0f, 1.0f));
            if (CurrentTimeOfDay > 18.0f)
            {
                SkyIntensity = FMath::Lerp(2.0f, 0.1f, (CurrentTimeOfDay - 18.0f) / 6.0f);
            }
            SkyComp->SetIntensity(SkyIntensity);
        }
    }
}

void ADayNightCycleManager::ApplyLightingSettings(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
        UDirectionalLightComponent* Comp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (Comp)
        {
            Comp->SetIntensity(Settings.SunIntensity);
            Comp->SetLightColor(Settings.SunColor);
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
        }
    }

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
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha)
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogInscatteringColor = FLinearColor(
        FMath::Lerp(A.FogInscatteringColor.R, B.FogInscatteringColor.R, Alpha),
        FMath::Lerp(A.FogInscatteringColor.G, B.FogInscatteringColor.G, Alpha),
        FMath::Lerp(A.FogInscatteringColor.B, B.FogInscatteringColor.B, Alpha)
    );
    return Result;
}

void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
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

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f) return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f) return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 2.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
}

void ADayNightCycleManager::JumpToTimePreset(ELight_TimeOfDay TimePreset)
{
    switch (TimePreset)
    {
    case ELight_TimeOfDay::Dawn:
        SetTimeOfDay(6.0f);
        ApplyLightingSettings(DawnSettings);
        break;
    case ELight_TimeOfDay::Morning:
        SetTimeOfDay(9.0f);
        break;
    case ELight_TimeOfDay::Midday:
        SetTimeOfDay(12.0f);
        ApplyLightingSettings(MiddaySettings);
        break;
    case ELight_TimeOfDay::Dusk:
        SetTimeOfDay(19.0f);
        ApplyLightingSettings(DuskSettings);
        break;
    case ELight_TimeOfDay::Night:
        SetTimeOfDay(22.0f);
        ApplyLightingSettings(NightSettings);
        break;
    default:
        break;
    }
}

FString ADayNightCycleManager::GetFormattedTime() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

bool ADayNightCycleManager::IsDaytime() const
{
    return CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay < 20.0f;
}

bool ADayNightCycleManager::IsNighttime() const
{
    return !IsDaytime();
}

float ADayNightCycleManager::GetCurrentSunIntensity() const
{
    return GetSunIntensityForTime(CurrentTimeOfDay);
}

float ADayNightCycleManager::GetSunPitchForTime(float Hour) const
{
    // Sun rises at 6am, sets at 18pm
    // At noon (12h) pitch = -75 (high in sky)
    // At dawn/dusk pitch = -5 (near horizon)
    // At night pitch = +30 (below horizon)
    if (Hour < 6.0f || Hour > 18.0f)
    {
        return 30.0f; // Below horizon at night
    }
    float DayProgress = (Hour - 6.0f) / 12.0f; // 0 to 1 over the day
    float SinValue = FMath::Sin(DayProgress * PI);
    return FMath::Lerp(-5.0f, -75.0f, SinValue);
}

float ADayNightCycleManager::GetSunYawForTime(float Hour) const
{
    // Sun moves from east (-90) to west (90) during the day
    if (Hour < 6.0f || Hour > 18.0f)
    {
        return 180.0f;
    }
    float DayProgress = (Hour - 6.0f) / 12.0f;
    return FMath::Lerp(-90.0f, 90.0f, DayProgress);
}

float ADayNightCycleManager::GetSunIntensityForTime(float Hour) const
{
    if (Hour < 5.0f || Hour > 19.0f) return 0.0f;
    if (Hour < 6.0f) return FMath::Lerp(0.0f, 2.0f, Hour - 5.0f);
    if (Hour > 18.0f) return FMath::Lerp(2.0f, 0.0f, Hour - 18.0f);

    float DayProgress = (Hour - 6.0f) / 12.0f;
    float SinValue = FMath::Sin(DayProgress * PI);
    return FMath::Lerp(2.0f, 12.0f, SinValue);
}

FLinearColor ADayNightCycleManager::GetSunColorForTime(float Hour) const
{
    // Dawn/dusk = warm orange, midday = white/yellow
    if (Hour < 6.0f || Hour > 18.0f)
    {
        return FLinearColor(0.1f, 0.15f, 0.3f, 1.0f); // Night blue
    }

    float DayProgress = (Hour - 6.0f) / 12.0f;
    float SinValue = FMath::Sin(DayProgress * PI);

    // Warm orange at dawn/dusk, white at noon
    FLinearColor WarmOrange(1.0f, 0.5f, 0.2f, 1.0f);
    FLinearColor NoonWhite(1.0f, 0.98f, 0.9f, 1.0f);
    return FLinearColor(
        FMath::Lerp(WarmOrange.R, NoonWhite.R, SinValue),
        FMath::Lerp(WarmOrange.G, NoonWhite.G, SinValue),
        FMath::Lerp(WarmOrange.B, NoonWhite.B, SinValue)
    );
}

FLinearColor ADayNightCycleManager::GetFogColorForTime(float Hour) const
{
    if (Hour < 6.0f || Hour > 18.0f)
    {
        return FLinearColor(0.05f, 0.08f, 0.2f, 1.0f); // Night dark blue
    }

    float DayProgress = (Hour - 6.0f) / 12.0f;
    float SinValue = FMath::Sin(DayProgress * PI);

    FLinearColor DawnFog(0.8f, 0.5f, 0.3f, 1.0f);
    FLinearColor DayFog(0.5f, 0.6f, 0.8f, 1.0f);
    return FLinearColor(
        FMath::Lerp(DawnFog.R, DayFog.R, SinValue),
        FMath::Lerp(DawnFog.G, DayFog.G, SinValue),
        FMath::Lerp(DawnFog.B, DayFog.B, SinValue)
    );
}

float ADayNightCycleManager::GetFogDensityForTime(float Hour) const
{
    // Denser fog at dawn/dusk, lighter at midday
    if (Hour < 6.0f || Hour > 18.0f) return 0.03f;

    float DayProgress = (Hour - 6.0f) / 12.0f;
    float SinValue = FMath::Sin(DayProgress * PI);
    return FMath::Lerp(0.04f, 0.01f, SinValue);
}
