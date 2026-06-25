#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth cycle

    // Default golden hour sun config
    GoldenHourSunConfig.Pitch = -38.0f;
    GoldenHourSunConfig.Yaw = 45.0f;
    GoldenHourSunConfig.Intensity = 12.0f;
    GoldenHourSunConfig.LightColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    GoldenHourSunConfig.bAtmosphereSunLight = true;

    // Default midday sun config
    MiddaySunConfig.Pitch = -75.0f;
    MiddaySunConfig.Yaw = 0.0f;
    MiddaySunConfig.Intensity = 15.0f;
    MiddaySunConfig.LightColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddaySunConfig.bAtmosphereSunLight = true;

    // Default clear weather fog
    ClearWeatherFog.FogDensity = 0.02f;
    ClearWeatherFog.FogHeightFalloff = 0.2f;
    ClearWeatherFog.FogInscatteringColor = FLinearColor(0.45f, 0.6f, 0.85f, 1.0f);
    ClearWeatherFog.bVolumetricFog = true;
    ClearWeatherFog.VolumetricFogExtinctionScale = 0.5f;

    // Default stormy weather fog
    StormyWeatherFog.FogDensity = 0.08f;
    StormyWeatherFog.FogHeightFalloff = 0.15f;
    StormyWeatherFog.FogInscatteringColor = FLinearColor(0.3f, 0.35f, 0.4f, 1.0f);
    StormyWeatherFog.bVolumetricFog = true;
    StormyWeatherFog.VolumetricFogExtinctionScale = 1.5f;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyGoldenHourPreset();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive)
    {
        UpdateSunPosition(DeltaTime);
        UpdateTimeOfDayEnum();
    }
}

void ACretaceousLightingManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }
    UpdateTimeOfDayEnum();
    UpdateSunPosition(0.0f);
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateFogForWeather();
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    CurrentTimeOfDay = 17.5f; // 5:30 PM golden hour
    CurrentWeather = ELight_WeatherState::Clear;

    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(GoldenHourSunConfig.Intensity);
            SunComp->SetLightColor(GoldenHourSunConfig.LightColor);
            SunComp->bAtmosphereSunLight = GoldenHourSunConfig.bAtmosphereSunLight;
        }
        SunLight->SetActorRotation(FRotator(GoldenHourSunConfig.Pitch, GoldenHourSunConfig.Yaw, 0.0f));
    }

    UpdateFogForWeather();
    UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Golden Hour preset applied"));
}

void ACretaceousLightingManager::ApplyMiddayPreset()
{
    CurrentTimeOfDay = 12.0f;
    CurrentWeather = ELight_WeatherState::Clear;

    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(MiddaySunConfig.Intensity);
            SunComp->SetLightColor(MiddaySunConfig.LightColor);
            SunComp->bAtmosphereSunLight = MiddaySunConfig.bAtmosphereSunLight;
        }
        SunLight->SetActorRotation(FRotator(MiddaySunConfig.Pitch, MiddaySunConfig.Yaw, 0.0f));
    }

    UpdateFogForWeather();
    UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Midday preset applied"));
}

void ACretaceousLightingManager::ApplyNightPreset()
{
    CurrentTimeOfDay = 0.0f; // Midnight
    CurrentWeather = ELight_WeatherState::Clear;

    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(0.1f); // Moon-like intensity
            SunComp->SetLightColor(FLinearColor(0.4f, 0.5f, 0.8f, 1.0f)); // Cool blue moonlight
        }
        SunLight->SetActorRotation(FRotator(-20.0f, 180.0f, 0.0f));
    }

    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(0.04f);
            FogComp->SetFogInscatteringColor(FLinearColor(0.05f, 0.08f, 0.15f, 1.0f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Night preset applied"));
}

void ACretaceousLightingManager::ApplyStormPreset()
{
    CurrentWeather = ELight_WeatherState::Storm;

    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(3.0f);
            SunComp->SetLightColor(FLinearColor(0.6f, 0.65f, 0.7f, 1.0f)); // Cold grey storm light
        }
    }

    UpdateFogForWeather();
    UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Storm preset applied"));
}

void ACretaceousLightingManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Found DirectionalLight: %s"), *SunLight->GetName());
    }

    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Found SkyLight: %s"), *SkyLightActor->GetName());
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Found ExponentialHeightFog: %s"), *HeightFogActor->GetName());
    }

    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolumeActor = Cast<APostProcessVolume>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLighting: Found PostProcessVolume: %s"), *PostProcessVolumeActor->GetName());
    }
}

FString ACretaceousLightingManager::GetTimeOfDayString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

float ACretaceousLightingManager::GetSunPitchForTime(float TimeOfDay) const
{
    // Sun rises at 6:00, sets at 18:00
    // Pitch: -90 at noon (straight up), +90 at midnight (below horizon)
    float NormalizedTime = (TimeOfDay - 6.0f) / 12.0f; // 0 at 6am, 1 at 6pm
    float SunAngle = NormalizedTime * 180.0f - 90.0f;   // -90 at 6am, 90 at 6pm
    return -SunAngle; // Negative because UE pitch convention
}

FLinearColor ACretaceousLightingManager::GetSunColorForTime(float TimeOfDay) const
{
    // Dawn/Dusk: warm orange-red
    // Midday: bright white-yellow
    // Night: cool blue (moon)

    if (TimeOfDay < 5.0f || TimeOfDay > 21.0f)
    {
        // Night — cool moonlight blue
        return FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    }
    else if (TimeOfDay < 7.0f)
    {
        // Dawn — warm orange
        float T = (TimeOfDay - 5.0f) / 2.0f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.8f, 0.3f, 0.1f, 1.0f), FLinearColor(1.0f, 0.85f, 0.6f, 1.0f), T);
    }
    else if (TimeOfDay < 10.0f)
    {
        // Morning — warm golden
        return FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);
    }
    else if (TimeOfDay < 14.0f)
    {
        // Midday — bright white-yellow
        return FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    }
    else if (TimeOfDay < 17.0f)
    {
        // Afternoon — warm yellow
        return FLinearColor(1.0f, 0.92f, 0.75f, 1.0f);
    }
    else if (TimeOfDay < 19.0f)
    {
        // Golden hour — deep amber
        return FLinearColor(1.0f, 0.65f, 0.3f, 1.0f);
    }
    else
    {
        // Dusk — red-orange fading
        float T = (TimeOfDay - 19.0f) / 2.0f;
        return FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.4f, 0.15f, 1.0f), FLinearColor(0.4f, 0.5f, 0.8f, 1.0f), T);
    }
}

void ACretaceousLightingManager::UpdateSunPosition(float DeltaTime)
{
    if (!SunLight) return;

    // Advance time
    if (DeltaTime > 0.0f)
    {
        float HoursPerSecond = DayCycleSpeed / 3600.0f;
        CurrentTimeOfDay += DeltaTime * HoursPerSecond;
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }

    // Calculate sun pitch
    float SunPitch = GetSunPitchForTime(CurrentTimeOfDay);
    float SunYaw = FMath::Lerp(0.0f, 360.0f, CurrentTimeOfDay / 24.0f);

    // Apply sun rotation
    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));

    // Apply sun color and intensity
    UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (SunComp)
    {
        FLinearColor SunColor = GetSunColorForTime(CurrentTimeOfDay);
        SunComp->SetLightColor(SunColor);

        // Intensity: 0 at night, 12 at golden hour, 15 at midday
        float IntensityMultiplier = 0.0f;
        if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 18.0f)
        {
            float DayProgress = (CurrentTimeOfDay - 6.0f) / 12.0f;
            IntensityMultiplier = FMath::Sin(DayProgress * PI);
        }
        SunComp->SetIntensity(FMath::Lerp(0.1f, 15.0f, IntensityMultiplier));
    }
}

void ACretaceousLightingManager::UpdateFogForWeather()
{
    if (!HeightFogActor) return;

    UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    switch (CurrentWeather)
    {
    case ELight_WeatherState::Clear:
        FogComp->SetFogDensity(ClearWeatherFog.FogDensity);
        FogComp->SetFogInscatteringColor(ClearWeatherFog.FogInscatteringColor);
        break;

    case ELight_WeatherState::PartlyCloudy:
        FogComp->SetFogDensity(0.03f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.5f, 0.65f, 0.85f, 1.0f));
        break;

    case ELight_WeatherState::Overcast:
        FogComp->SetFogDensity(0.05f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.4f, 0.45f, 0.55f, 1.0f));
        break;

    case ELight_WeatherState::Rain:
        FogComp->SetFogDensity(0.06f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.35f, 0.4f, 0.5f, 1.0f));
        break;

    case ELight_WeatherState::Storm:
        FogComp->SetFogDensity(StormyWeatherFog.FogDensity);
        FogComp->SetFogInscatteringColor(StormyWeatherFog.FogInscatteringColor);
        break;

    case ELight_WeatherState::Fog:
        FogComp->SetFogDensity(0.12f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.75f, 0.8f, 1.0f));
        break;

    case ELight_WeatherState::HeatHaze:
        FogComp->SetFogDensity(0.015f);
        FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.65f, 0.5f, 1.0f));
        break;

    default:
        break;
    }
}

void ACretaceousLightingManager::UpdateTimeOfDayEnum()
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 10.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 14.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::GoldenHour;
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 21.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 21.0f || CurrentTimeOfDay < 2.0f)
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Night;
    else
        CurrentTimeOfDayEnum = ELight_TimeOfDay::Midnight;
}
