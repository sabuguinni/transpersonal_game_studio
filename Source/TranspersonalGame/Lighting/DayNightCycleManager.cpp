#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default presets — prehistoric Cretaceous atmosphere
    // Dawn: cool blue-pink, low sun, dense fog
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunPitch = -5.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.5f, 0.5f, 0.7f, 1.0f);
    DawnSettings.ExposureBias = 0.5f;

    // Midday: bright white sun, clear sky, minimal fog
    MiddaySettings.SunIntensity = 12.0f;
    MiddaySettings.SunPitch = -75.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddaySettings.SkyLightIntensity = 2.0f;
    MiddaySettings.FogDensity = 0.01f;
    MiddaySettings.FogColor = FLinearColor(0.5f, 0.65f, 0.9f, 1.0f);
    MiddaySettings.ExposureBias = 1.2f;

    // Dusk: warm amber/orange, low sun, medium fog
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.7f, 0.4f, 0.2f, 1.0f);
    DuskSettings.ExposureBias = 0.6f;

    // Night: moonlight blue, stars, thick fog
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunPitch = -15.0f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    NightSettings.ExposureBias = -1.0f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    UpdateLightingFromTime();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleEnabled) return;

    ElapsedSeconds += DeltaTime;
    // Convert real seconds to game hours
    // TimeSpeedMultiplier=60 means 1 real second = 1 game minute
    float GameHoursPerRealSecond = TimeSpeedMultiplier / 3600.0f;
    CurrentTimeHours += DeltaTime * GameHoursPerRealSecond;

    if (CurrentTimeHours >= 24.0f)
    {
        CurrentTimeHours -= 24.0f;
    }

    CurrentTimeOfDay = TimeToEnum(CurrentTimeHours);
    UpdateSunPosition();
    UpdateLightingFromTime();
}

void ADayNightCycleManager::SetTimeOfDay(float NewTimeHours)
{
    CurrentTimeHours = FMath::Clamp(NewTimeHours, 0.0f, 24.0f);
    CurrentTimeOfDay = TimeToEnum(CurrentTimeHours);
    AutoFindLightActors();
    UpdateSunPosition();
    UpdateLightingFromTime();
}

FText ADayNightCycleManager::GetTimeDisplayString() const
{
    int32 Hours = FMath::FloorToInt(CurrentTimeHours);
    int32 Minutes = FMath::FloorToInt((CurrentTimeHours - Hours) * 60.0f);
    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Hours, Minutes));
}

void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunActor = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    if (!FogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ADayNightCycleManager::UpdateSunPosition()
{
    if (!SunActor) return;

    // Map time 0-24 to sun pitch angle
    // 6am = sunrise (pitch -5), 12pm = zenith (pitch -75), 6pm = sunset (pitch -5), night = below horizon
    float NormalizedTime = CurrentTimeHours / 24.0f;
    float SunAngle = NormalizedTime * 360.0f - 90.0f; // 0h = -90deg, 12h = 90deg
    float SunPitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;

    // Ensure pitch is always negative (sun points down at terrain)
    SunPitch = FMath::Clamp(SunPitch, -80.0f, -2.0f);

    // Yaw rotates around the world (east to west)
    float SunYaw = 45.0f + NormalizedTime * 180.0f;

    SunActor->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));
}

void ADayNightCycleManager::UpdateLightingFromTime()
{
    FLight_TimeOfDaySettings CurrentSettings;

    // Blend between presets based on time
    if (CurrentTimeHours < 6.0f)
    {
        // Night to Dawn
        float Alpha = FMath::Clamp((CurrentTimeHours - 4.0f) / 2.0f, 0.0f, 1.0f);
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (CurrentTimeHours < 10.0f)
    {
        // Dawn to Midday
        float Alpha = FMath::Clamp((CurrentTimeHours - 6.0f) / 4.0f, 0.0f, 1.0f);
        CurrentSettings = InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (CurrentTimeHours < 16.0f)
    {
        // Midday plateau
        CurrentSettings = MiddaySettings;
    }
    else if (CurrentTimeHours < 20.0f)
    {
        // Midday to Dusk
        float Alpha = FMath::Clamp((CurrentTimeHours - 16.0f) / 4.0f, 0.0f, 1.0f);
        CurrentSettings = InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (CurrentTimeHours < 22.0f)
    {
        // Dusk to Night
        float Alpha = FMath::Clamp((CurrentTimeHours - 20.0f) / 2.0f, 0.0f, 1.0f);
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Full night
        CurrentSettings = NightSettings;
    }

    // Apply to DirectionalLight (Sun)
    if (SunActor)
    {
        UDirectionalLightComponent* DLComp = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (DLComp)
        {
            DLComp->SetIntensity(CurrentSettings.SunIntensity);
            DLComp->SetLightColor(CurrentSettings.SunColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLComp)
        {
            SLComp->SetIntensity(CurrentSettings.SkyLightIntensity);
        }
    }

    // Apply to Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(CurrentSettings.FogDensity);
            FogComp->SetFogInscatteringColor(CurrentSettings.FogColor);
        }
    }
}

ELight_TimeOfDay ADayNightCycleManager::TimeToEnum(float Hours) const
{
    if (Hours < 5.0f)  return ELight_TimeOfDay::Midnight;
    if (Hours < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (Hours < 10.0f) return ELight_TimeOfDay::Morning;
    if (Hours < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours < 20.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDaySettings ADayNightCycleManager::InterpolateSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}
