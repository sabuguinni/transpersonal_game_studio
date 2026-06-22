#include "DayNightCycleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    InitDefaultPresets();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightActorsInWorld();
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleEnabled || DayDurationSeconds <= 0.0f)
        return;

    // Advance time: 24 hours over DayDurationSeconds
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;

    if (CurrentTimeOfDay >= 24.0f)
        CurrentTimeOfDay -= 24.0f;

    UpdateSunPosition(CurrentTimeOfDay);
    UpdateLightColors(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    CurrentPhase = TimeToPhase(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition(CurrentTimeOfDay);
    UpdateLightColors(CurrentTimeOfDay);
    UpdateFogSettings(CurrentTimeOfDay);
    CurrentPhase = TimeToPhase(CurrentTimeOfDay);
}

void ADayNightCycleManager::FindLightActorsInWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;

    // Find DirectionalLight (Sun)
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UClass* FogClass = FindObject<UClass>(ANY_PACKAGE, TEXT("ExponentialHeightFog"));
    if (FogClass)
    {
        UGameplayStatics::GetAllActorsOfClass(World, FogClass, FoundActors);
        if (FoundActors.Num() > 0)
            FogActor = FoundActors[0];
    }

    // Find SkyLight
    FoundActors.Empty();
    UClass* SkyLightClass = FindObject<UClass>(ANY_PACKAGE, TEXT("SkyLight"));
    if (SkyLightClass)
    {
        UGameplayStatics::GetAllActorsOfClass(World, SkyLightClass, FoundActors);
        if (FoundActors.Num() > 0)
            SkyLightActor = FoundActors[0];
    }
}

void ADayNightCycleManager::UpdateSunPosition(float TimeHours)
{
    if (!SunLight) return;

    // Map 0-24h to sun arc: rises at 6h, sets at 18h
    // Pitch: -90 at noon (straight down), +90 at midnight (below horizon)
    float NormalizedTime = (TimeHours - 6.0f) / 12.0f; // 0=6am, 1=6pm
    float SunPitch = FMath::Lerp(-10.0f, -170.0f, NormalizedTime);

    // Clamp so sun doesn't go below horizon in a jarring way
    SunPitch = FMath::Clamp(SunPitch, -175.0f, -5.0f);

    FRotator NewRot(SunPitch, 45.0f, 0.0f);
    SunLight->SetActorRotation(NewRot);
}

void ADayNightCycleManager::UpdateLightColors(float TimeHours)
{
    if (!SunLight) return;

    UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!DLC) return;

    // Define color keyframes
    // Dawn (6h): warm orange-pink
    // Midday (12h): neutral white-yellow
    // Dusk (18h): deep orange-red
    // Night (0h/24h): very dim blue

    FLinearColor SunColor;
    float Intensity;

    if (TimeHours < 6.0f) // Pre-dawn / night
    {
        float T = TimeHours / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(0.1f, 0.1f, 0.3f, 1.0f), FLinearColor(1.0f, 0.5f, 0.2f, 1.0f), T);
        Intensity = FMath::Lerp(0.1f, 5.0f, T);
    }
    else if (TimeHours < 12.0f) // Morning to midday
    {
        float T = (TimeHours - 6.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.6f, 0.3f, 1.0f), FLinearColor(1.0f, 0.97f, 0.88f, 1.0f), T);
        Intensity = FMath::Lerp(5.0f, 10.0f, T);
    }
    else if (TimeHours < 18.0f) // Midday to dusk
    {
        float T = (TimeHours - 12.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.97f, 0.88f, 1.0f), FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), T);
        Intensity = FMath::Lerp(10.0f, 4.0f, T);
    }
    else // Dusk to night
    {
        float T = (TimeHours - 18.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), FLinearColor(0.1f, 0.1f, 0.3f, 1.0f), T);
        Intensity = FMath::Lerp(4.0f, 0.1f, T);
    }

    DLC->SetIntensity(Intensity);
    DLC->SetLightColor(SunColor);
}

void ADayNightCycleManager::UpdateFogSettings(float TimeHours)
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    // Dawn/Dusk: thicker fog; Midday: lighter; Night: medium blue
    float FogDensity;
    FLinearColor FogColor;

    if (TimeHours < 6.0f || TimeHours > 20.0f) // Night
    {
        FogDensity = 0.03f;
        FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    }
    else if (TimeHours < 8.0f || TimeHours > 18.0f) // Dawn/Dusk
    {
        FogDensity = 0.04f;
        FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else // Day
    {
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
    }

    FogComp->SetFogDensity(FogDensity);
    FogComp->SetFogInscatteringColor(FogColor);
}

ELight_TimeOfDay ADayNightCycleManager::TimeToPhase(float TimeHours) const
{
    if (TimeHours < 6.0f)  return ELight_TimeOfDay::Night;
    if (TimeHours < 8.0f)  return ELight_TimeOfDay::Dawn;
    if (TimeHours < 11.0f) return ELight_TimeOfDay::Morning;
    if (TimeHours < 14.0f) return ELight_TimeOfDay::Midday;
    if (TimeHours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (TimeHours < 20.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDaySettings ADayNightCycleManager::LerpSettings(
    const FLight_TimeOfDaySettings& A,
    const FLight_TimeOfDaySettings& B,
    float Alpha) const
{
    FLight_TimeOfDaySettings Result;
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings)
{
    if (SunLight)
    {
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Settings.SunIntensity);
            DLC->SetLightColor(Settings.SunColor);
        }
        SunLight->SetActorRotation(FRotator(Settings.SunPitch, Settings.SunYaw, 0.0f));
    }

    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogColor);
        }
    }
}

void ADayNightCycleManager::InitDefaultPresets()
{
    // Dawn
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnSettings.SunPitch = -10.0f;
    DawnSettings.SunYaw = -90.0f;

    // Midday
    MiddaySettings.SunIntensity = 10.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.5f;
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);
    MiddaySettings.SunPitch = -80.0f;
    MiddaySettings.SunYaw = 0.0f;

    // Dusk
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.7f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SunPitch = -8.0f;
    DuskSettings.SunYaw = 90.0f;

    // Night
    NightSettings.SunIntensity = 0.05f;
    NightSettings.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
    NightSettings.SunPitch = 45.0f;
    NightSettings.SunYaw = 180.0f;
}
