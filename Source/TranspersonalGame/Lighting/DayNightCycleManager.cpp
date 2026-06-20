#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second for performance

    CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    CurrentWeather = ELight_WeatherState::Clear;
    CurrentHour = 10.0f;
    SecondsPerHour = 60.0f;
    bCycleEnabled = true;

    // Initialize default presets for Cretaceous prehistoric world
    // Dawn — cool blue-pink light, heavy mist
    FLight_TimeOfDayPreset DawnPreset;
    DawnPreset.TimeOfDay = ELight_TimeOfDay::Dawn;
    DawnPreset.SunSettings.Intensity = 3.0f;
    DawnPreset.SunSettings.LightColor = FLinearColor(1.0f, 0.75f, 0.55f, 1.0f);
    DawnPreset.SunSettings.SunPitch = -5.0f;
    DawnPreset.SunSettings.SunYaw = -90.0f;
    DawnPreset.FogSettings.FogDensity = 0.06f;
    DawnPreset.FogSettings.FogAlbedo = FLinearColor(0.75f, 0.80f, 0.95f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.5f;
    TimeOfDayPresets.Add(DawnPreset);

    // Morning — warm golden light
    FLight_TimeOfDayPreset MorningPreset;
    MorningPreset.TimeOfDay = ELight_TimeOfDay::Morning;
    MorningPreset.SunSettings.Intensity = 8.0f;
    MorningPreset.SunSettings.LightColor = FLinearColor(1.0f, 0.90f, 0.70f, 1.0f);
    MorningPreset.SunSettings.SunPitch = -25.0f;
    MorningPreset.SunSettings.SunYaw = -60.0f;
    MorningPreset.FogSettings.FogDensity = 0.03f;
    MorningPreset.FogSettings.FogAlbedo = FLinearColor(0.85f, 0.88f, 0.92f, 1.0f);
    MorningPreset.SkyLightIntensity = 1.0f;
    TimeOfDayPresets.Add(MorningPreset);

    // Midday — bright white light, minimal fog
    FLight_TimeOfDayPreset MiddayPreset;
    MiddayPreset.TimeOfDay = ELight_TimeOfDay::Midday;
    MiddayPreset.SunSettings.Intensity = 12.0f;
    MiddayPreset.SunSettings.LightColor = FLinearColor(1.0f, 0.97f, 0.92f, 1.0f);
    MiddayPreset.SunSettings.SunPitch = -75.0f;
    MiddayPreset.SunSettings.SunYaw = 0.0f;
    MiddayPreset.FogSettings.FogDensity = 0.01f;
    MiddayPreset.FogSettings.FogAlbedo = FLinearColor(0.90f, 0.92f, 0.95f, 1.0f);
    MiddayPreset.SkyLightIntensity = 2.0f;
    TimeOfDayPresets.Add(MiddayPreset);

    // Afternoon — warm amber, long shadows
    FLight_TimeOfDayPreset AfternoonPreset;
    AfternoonPreset.TimeOfDay = ELight_TimeOfDay::Afternoon;
    AfternoonPreset.SunSettings.Intensity = 10.0f;
    AfternoonPreset.SunSettings.LightColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    AfternoonPreset.SunSettings.SunPitch = -35.0f;
    AfternoonPreset.SunSettings.SunYaw = 60.0f;
    AfternoonPreset.FogSettings.FogDensity = 0.02f;
    AfternoonPreset.FogSettings.FogAlbedo = FLinearColor(0.88f, 0.85f, 0.80f, 1.0f);
    AfternoonPreset.SkyLightIntensity = 1.5f;
    TimeOfDayPresets.Add(AfternoonPreset);

    // Dusk — deep orange-red, heavy atmosphere
    FLight_TimeOfDayPreset DuskPreset;
    DuskPreset.TimeOfDay = ELight_TimeOfDay::Dusk;
    DuskPreset.SunSettings.Intensity = 4.0f;
    DuskPreset.SunSettings.LightColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    DuskPreset.SunSettings.SunPitch = -5.0f;
    DuskPreset.SunSettings.SunYaw = 90.0f;
    DuskPreset.FogSettings.FogDensity = 0.05f;
    DuskPreset.FogSettings.FogAlbedo = FLinearColor(0.90f, 0.70f, 0.55f, 1.0f);
    DuskPreset.SkyLightIntensity = 0.4f;
    TimeOfDayPresets.Add(DuskPreset);

    // Night — moonlight, deep blue
    FLight_TimeOfDayPreset NightPreset;
    NightPreset.TimeOfDay = ELight_TimeOfDay::Night;
    NightPreset.SunSettings.Intensity = 0.5f;
    NightPreset.SunSettings.LightColor = FLinearColor(0.55f, 0.65f, 0.85f, 1.0f);
    NightPreset.SunSettings.SunPitch = -10.0f;
    NightPreset.SunSettings.SunYaw = 180.0f;
    NightPreset.FogSettings.FogDensity = 0.04f;
    NightPreset.FogSettings.FogAlbedo = FLinearColor(0.30f, 0.35f, 0.55f, 1.0f);
    NightPreset.SkyLightIntensity = 0.2f;
    TimeOfDayPresets.Add(NightPreset);
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    ApplyCurrentSettings();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleEnabled) return;

    ElapsedSeconds += DeltaTime;
    if (ElapsedSeconds >= SecondsPerHour)
    {
        ElapsedSeconds -= SecondsPerHour;
        CurrentHour += 1.0f;
        if (CurrentHour >= 24.0f)
        {
            CurrentHour -= 24.0f;
        }
        SetTimeOfDay(CurrentHour);
    }
}

void ADayNightCycleManager::SetTimeOfDay(float Hour)
{
    CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    CurrentTimeOfDay = GetTimeOfDayEnum();

    UpdateSunPosition(CurrentHour);
    UpdateFogForTimeOfDay(CurrentHour);
    UpdateSkyLightForTimeOfDay(CurrentHour);
    ApplyWeatherOverlay();
}

void ADayNightCycleManager::SetWeather(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    ApplyWeatherOverlay();
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayEnum() const
{
    if (CurrentHour >= 5.0f && CurrentHour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (CurrentHour >= 7.0f && CurrentHour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (CurrentHour >= 11.0f && CurrentHour < 14.0f) return ELight_TimeOfDay::Midday;
    if (CurrentHour >= 14.0f && CurrentHour < 18.0f) return ELight_TimeOfDay::Afternoon;
    if (CurrentHour >= 18.0f && CurrentHour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (CurrentHour >= 20.0f || CurrentHour < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return (CurrentHour >= 20.0f || CurrentHour < 5.0f);
}

bool ADayNightCycleManager::IsDawnOrDusk() const
{
    return (CurrentHour >= 5.0f && CurrentHour < 7.0f) ||
           (CurrentHour >= 18.0f && CurrentHour < 20.0f);
}

void ADayNightCycleManager::UpdateSunPosition(float Hour)
{
    if (!SunLight) return;

    FLight_SunSettings Settings = GetSunSettingsForHour(Hour);

    SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));

    UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DLC)
    {
        DLC->SetIntensity(Settings.Intensity);
        DLC->SetLightColor(Settings.LightColor);
    }
}

void ADayNightCycleManager::UpdateFogForTimeOfDay(float Hour)
{
    if (!HeightFog) return;

    FLight_FogSettings FogSettings = GetFogSettingsForHour(Hour);

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(FogSettings.FogDensity);
        FogComp->SetFogHeightFalloff(FogSettings.FogHeightFalloff);
        FogComp->SetFogMaxOpacity(FogSettings.FogMaxOpacity);
        FogComp->SetVolumetricFog(FogSettings.bVolumetricFog);
    }
}

void ADayNightCycleManager::UpdateSkyLightForTimeOfDay(float Hour)
{
    if (!SkyLightActor) return;

    // Interpolate sky light intensity based on time
    float SkyIntensity = 1.5f;
    if (IsNightTime())
    {
        SkyIntensity = 0.2f;
    }
    else if (IsDawnOrDusk())
    {
        SkyIntensity = 0.5f;
    }
    else if (CurrentTimeOfDay == ELight_TimeOfDay::Midday)
    {
        SkyIntensity = 2.0f;
    }

    USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
    if (SLC)
    {
        SLC->SetIntensity(SkyIntensity);
    }
}

void ADayNightCycleManager::ApplyWeatherOverlay()
{
    if (!HeightFog) return;

    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    switch (CurrentWeather)
    {
    case ELight_WeatherState::Overcast:
        FogComp->SetFogDensity(FogComp->FogDensity * 2.0f);
        break;
    case ELight_WeatherState::Rain:
        FogComp->SetFogDensity(0.08f);
        FogComp->SetFogMaxOpacity(0.95f);
        break;
    case ELight_WeatherState::Storm:
        FogComp->SetFogDensity(0.12f);
        FogComp->SetFogMaxOpacity(1.0f);
        break;
    case ELight_WeatherState::Fog:
        FogComp->SetFogDensity(0.15f);
        FogComp->SetFogHeightFalloff(0.05f);
        break;
    case ELight_WeatherState::Haze:
        FogComp->SetFogDensity(0.04f);
        break;
    case ELight_WeatherState::Clear:
    default:
        break;
    }
}

void ADayNightCycleManager::ApplyCurrentSettings()
{
    SetTimeOfDay(CurrentHour);
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

FLight_SunSettings ADayNightCycleManager::GetSunSettingsForHour(float Hour) const
{
    // Map hour to sun pitch: 6am = -5deg, 12pm = -75deg, 6pm = -5deg, night = -10deg (moon)
    float NormalizedDay = FMath::Clamp((Hour - 6.0f) / 12.0f, 0.0f, 1.0f);
    float SunPitch = FMath::Lerp(-5.0f, -75.0f, FMath::Sin(NormalizedDay * PI));

    // Yaw sweeps from east to west
    float SunYaw = FMath::Lerp(-90.0f, 90.0f, NormalizedDay);

    // Intensity peaks at noon
    float Intensity = 12.0f * FMath::Max(0.0f, FMath::Sin(NormalizedDay * PI));
    if (IsNightTime()) Intensity = 0.5f;

    // Color shifts: dawn=orange, midday=white, dusk=red
    FLinearColor Color;
    if (Hour < 7.0f || Hour > 19.0f)
    {
        Color = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Dawn/Dusk orange
    }
    else if (Hour > 11.0f && Hour < 14.0f)
    {
        Color = FLinearColor(1.0f, 0.97f, 0.92f, 1.0f); // Midday white
    }
    else
    {
        Color = FLinearColor(1.0f, 0.90f, 0.72f, 1.0f); // Morning/Afternoon warm
    }

    FLight_SunSettings Result;
    Result.Intensity = Intensity;
    Result.LightColor = Color;
    Result.SunPitch = SunPitch;
    Result.SunYaw = SunYaw;
    return Result;
}

FLight_FogSettings ADayNightCycleManager::GetFogSettingsForHour(float Hour) const
{
    FLight_FogSettings Result;
    Result.bVolumetricFog = true;
    Result.VolumetricFogDistance = 6000.0f;
    Result.FogHeightFalloff = 0.2f;
    Result.FogMaxOpacity = 0.85f;

    if (Hour >= 5.0f && Hour < 8.0f)
    {
        // Dawn — heavy morning mist
        Result.FogDensity = 0.06f;
        Result.FogAlbedo = FLinearColor(0.75f, 0.80f, 0.95f, 1.0f);
    }
    else if (Hour >= 11.0f && Hour < 14.0f)
    {
        // Midday — clear
        Result.FogDensity = 0.01f;
        Result.FogAlbedo = FLinearColor(0.90f, 0.92f, 0.95f, 1.0f);
    }
    else if (Hour >= 18.0f && Hour < 21.0f)
    {
        // Dusk — warm haze
        Result.FogDensity = 0.05f;
        Result.FogAlbedo = FLinearColor(0.90f, 0.70f, 0.55f, 1.0f);
    }
    else if (Hour >= 21.0f || Hour < 5.0f)
    {
        // Night — cool blue mist
        Result.FogDensity = 0.04f;
        Result.FogAlbedo = FLinearColor(0.30f, 0.35f, 0.55f, 1.0f);
    }
    else
    {
        // Default daytime
        Result.FogDensity = 0.02f;
        Result.FogAlbedo = FLinearColor(0.85f, 0.88f, 0.92f, 1.0f);
    }

    return Result;
}

FLight_SunSettings ADayNightCycleManager::InterpolateSunSettings(
    const FLight_SunSettings& A, const FLight_SunSettings& B, float Alpha) const
{
    FLight_SunSettings Result;
    Result.Intensity = FMath::Lerp(A.Intensity, B.Intensity, Alpha);
    Result.LightColor = FLinearColor(
        FMath::Lerp(A.LightColor.R, B.LightColor.R, Alpha),
        FMath::Lerp(A.LightColor.G, B.LightColor.G, Alpha),
        FMath::Lerp(A.LightColor.B, B.LightColor.B, Alpha),
        1.0f
    );
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    return Result;
}

FLight_FogSettings ADayNightCycleManager::InterpolateFogSettings(
    const FLight_FogSettings& A, const FLight_FogSettings& B, float Alpha) const
{
    FLight_FogSettings Result;
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogMaxOpacity = FMath::Lerp(A.FogMaxOpacity, B.FogMaxOpacity, Alpha);
    Result.bVolumetricFog = A.bVolumetricFog;
    Result.VolumetricFogDistance = FMath::Lerp(A.VolumetricFogDistance, B.VolumetricFogDistance, Alpha);
    Result.FogAlbedo = FLinearColor(
        FMath::Lerp(A.FogAlbedo.R, B.FogAlbedo.R, Alpha),
        FMath::Lerp(A.FogAlbedo.G, B.FogAlbedo.G, Alpha),
        FMath::Lerp(A.FogAlbedo.B, B.FogAlbedo.B, Alpha),
        1.0f
    );
    return Result;
}
