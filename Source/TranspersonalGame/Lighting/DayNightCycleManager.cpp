#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    // Dawn defaults — warm pink/orange prehistoric dawn
    DawnSettings.SunPitch = -8.0f;
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.5f, 1.0f);
    DawnSettings.AmbientTemperature = 18.0f;

    // Midday defaults — bright harsh prehistoric sun
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.5f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    MiddaySettings.AmbientTemperature = 32.0f;

    // Dusk defaults — deep amber/red prehistoric sunset
    DuskSettings.SunPitch = -5.0f;
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.FogDensity = 0.06f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DuskSettings.AmbientTemperature = 24.0f;

    // Night defaults — cool blue prehistoric night
    NightSettings.SunPitch = 30.0f; // Sun below horizon
    NightSettings.SunIntensity = 0.05f;
    NightSettings.SunColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.FogDensity = 0.04f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.0f);
    NightSettings.AmbientTemperature = 14.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lights in the world if not manually assigned
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }

    if (!HeightFogActor)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            HeightFogActor = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }

    // Apply initial lighting for current time
    UpdateLighting(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time
    float HoursPerSecond = TimeSpeed / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    UpdateLighting(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting(CurrentTimeOfDay);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDayEnum() const
{
    float T = CurrentTimeOfDay;
    if (T >= 5.0f && T < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (T >= 7.0f && T < 11.0f)  return ELight_TimeOfDay::Morning;
    if (T >= 11.0f && T < 13.0f) return ELight_TimeOfDay::Midday;
    if (T >= 13.0f && T < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (T >= 17.0f && T < 20.0f) return ELight_TimeOfDay::Dusk;
    if (T >= 20.0f && T < 23.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

float ADayNightCycleManager::GetSunPitchForTime(float TimeHours) const
{
    // Sun rises at 6:00, peaks at 12:00, sets at 18:00
    // Pitch: -90 at horizon, -75 at zenith (UE convention: negative = looking down = sun high)
    float NormalizedTime = (TimeHours - 6.0f) / 12.0f; // 0 at 6am, 1 at 6pm
    NormalizedTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);

    // Sine curve: 0 at dawn/dusk, 1 at noon
    float SinValue = FMath::Sin(NormalizedTime * PI);

    // Map to pitch: -5 at horizon (dawn/dusk), -75 at zenith (noon)
    float Pitch = FMath::Lerp(-5.0f, -75.0f, SinValue);

    // Night: sun below horizon
    if (TimeHours < 5.0f || TimeHours > 19.0f)
    {
        Pitch = 20.0f; // Positive = sun below horizon
    }

    return Pitch;
}

FLinearColor ADayNightCycleManager::GetSunColorForTime(float TimeHours) const
{
    // Dawn: warm pink-orange
    if (TimeHours >= 5.0f && TimeHours < 7.0f)
    {
        float Alpha = (TimeHours - 5.0f) / 2.0f;
        return FLinearColor::LerpUsingHSV(DawnSettings.SunColor, MiddaySettings.SunColor, Alpha * 0.3f);
    }
    // Morning to midday: warm to neutral white
    if (TimeHours >= 7.0f && TimeHours < 12.0f)
    {
        float Alpha = (TimeHours - 7.0f) / 5.0f;
        return FLinearColor::LerpUsingHSV(DawnSettings.SunColor, MiddaySettings.SunColor, Alpha);
    }
    // Midday to afternoon
    if (TimeHours >= 12.0f && TimeHours < 17.0f)
    {
        float Alpha = (TimeHours - 12.0f) / 5.0f;
        return FLinearColor::LerpUsingHSV(MiddaySettings.SunColor, DuskSettings.SunColor, Alpha);
    }
    // Dusk: deep amber/red
    if (TimeHours >= 17.0f && TimeHours < 20.0f)
    {
        float Alpha = (TimeHours - 17.0f) / 3.0f;
        return FLinearColor::LerpUsingHSV(DuskSettings.SunColor, NightSettings.SunColor, Alpha);
    }
    return NightSettings.SunColor;
}

float ADayNightCycleManager::GetFogDensityForTime(float TimeHours) const
{
    // More fog at dawn/dusk, less at midday
    if (TimeHours >= 5.0f && TimeHours < 8.0f)
        return FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, (TimeHours - 5.0f) / 3.0f);
    if (TimeHours >= 8.0f && TimeHours < 16.0f)
        return MiddaySettings.FogDensity;
    if (TimeHours >= 16.0f && TimeHours < 20.0f)
        return FMath::Lerp(MiddaySettings.FogDensity, DuskSettings.FogDensity, (TimeHours - 16.0f) / 4.0f);
    return NightSettings.FogDensity;
}

FLight_TimeOfDaySettings ADayNightCycleManager::GetCurrentSettings() const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = GetSunPitchForTime(CurrentTimeOfDay);
    Result.SunColor = GetSunColorForTime(CurrentTimeOfDay);
    Result.FogDensity = GetFogDensityForTime(CurrentTimeOfDay);

    // Interpolate intensity
    float NormalizedTime = (CurrentTimeOfDay - 6.0f) / 12.0f;
    NormalizedTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    float SinValue = FMath::Sin(NormalizedTime * PI);
    Result.SunIntensity = FMath::Lerp(NightSettings.SunIntensity, MiddaySettings.SunIntensity, SinValue);
    Result.SkyLightIntensity = FMath::Lerp(NightSettings.SkyLightIntensity, MiddaySettings.SkyLightIntensity, SinValue);

    return Result;
}

FLight_TimeOfDaySettings ADayNightCycleManager::LerpSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
    return Result;
}

void ADayNightCycleManager::UpdateLighting(float TimeHours)
{
    FLight_TimeOfDaySettings CurrentSettings = GetCurrentSettings();
    ApplySettingsToActors(CurrentSettings);
}

void ADayNightCycleManager::ApplySettingsToActors(const FLight_TimeOfDaySettings& Settings)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, CurrentRot.Yaw, CurrentRot.Roll));

        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor.ToFColor(true));
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Settings.SkyLightIntensity);
        }
    }

    // Apply to height fog
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor.ToFColor(true));
        }
    }
}
