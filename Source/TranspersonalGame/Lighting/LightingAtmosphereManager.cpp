#include "LightingAtmosphereManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "EngineUtils.h"

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth transitions

    InitializeDefaultPalettes();
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    FindSceneLights();
    SetTimeOfDay(CurrentTimeOfDay);
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bStormActive)
    {
        StormRemainingTime -= DeltaTime;
        if (StormRemainingTime <= 0.0f)
        {
            bStormActive = false;
            SetTimeOfDay(CurrentTimeOfDay);
        }
    }
    else if (bDayCycleActive)
    {
        UpdateDayCycle(DeltaTime);
    }
}

void ALightingAtmosphereManager::FindSceneLights()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (Sun)
    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFog = *It;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: Found Sun=%s, Fog=%s, SkyLight=%s"),
        SunLight ? *SunLight->GetName() : TEXT("NONE"),
        HeightFog ? *HeightFog->GetName() : TEXT("NONE"),
        SkyLightActor ? *SkyLightActor->GetName() : TEXT("NONE"));
}

void ALightingAtmosphereManager::ApplyPalette(const FLight_TimeOfDayPalette& Palette)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
        SunLight->SetActorRotation(FRotator(Palette.SunPitch, Palette.SunYaw, 0.0f));
    }

    // Apply to ExponentialHeightFog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }
}

void ALightingAtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    FLight_TimeOfDayPalette Palette = GetPaletteForTime(NewTimeOfDay);
    ApplyPalette(Palette);

    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: TimeOfDay set to %d"), (int32)NewTimeOfDay);
}

FLight_TimeOfDayPalette ALightingAtmosphereManager::GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Morning:
            return DawnPalette;
        case ELight_TimeOfDay::Midday:
        case ELight_TimeOfDay::Afternoon:
            return MiddayPalette;
        case ELight_TimeOfDay::Dusk:
            return DuskPalette;
        case ELight_TimeOfDay::Night:
            return NightPalette;
        case ELight_TimeOfDay::Stormy:
            return StormyPalette;
        default:
            return MiddayPalette;
    }
}

void ALightingAtmosphereManager::TriggerStorm(float DurationSeconds)
{
    bStormActive = true;
    StormRemainingTime = DurationSeconds;
    ApplyPalette(StormyPalette);
    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: Storm triggered for %.1f seconds"), DurationSeconds);
}

void ALightingAtmosphereManager::UpdateDayCycle(float DeltaTime)
{
    if (DayCycleDurationSeconds <= 0.0f) return;

    NormalizedTimeOfDay += DeltaTime / DayCycleDurationSeconds;
    if (NormalizedTimeOfDay > 1.0f) NormalizedTimeOfDay -= 1.0f;

    ELight_TimeOfDay NewTime = GetTimeOfDayFromNormalized(NormalizedTimeOfDay);
    if (NewTime != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTime);
    }

    // Smoothly interpolate sun pitch based on normalized time
    // 0.0=midnight(pitch=+10), 0.25=dawn(pitch=-5), 0.5=midday(pitch=-75), 0.75=dusk(pitch=-18)
    float SunPitch = 0.0f;
    if (NormalizedTimeOfDay < 0.25f)
    {
        // Midnight to Dawn
        float t = NormalizedTimeOfDay / 0.25f;
        SunPitch = FMath::Lerp(10.0f, -5.0f, t);
    }
    else if (NormalizedTimeOfDay < 0.5f)
    {
        // Dawn to Midday
        float t = (NormalizedTimeOfDay - 0.25f) / 0.25f;
        SunPitch = FMath::Lerp(-5.0f, -75.0f, t);
    }
    else if (NormalizedTimeOfDay < 0.75f)
    {
        // Midday to Dusk
        float t = (NormalizedTimeOfDay - 0.5f) / 0.25f;
        SunPitch = FMath::Lerp(-75.0f, -18.0f, t);
    }
    else
    {
        // Dusk to Midnight
        float t = (NormalizedTimeOfDay - 0.75f) / 0.25f;
        SunPitch = FMath::Lerp(-18.0f, 10.0f, t);
    }

    if (SunLight)
    {
        FRotator CurrentRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(SunPitch, CurrentRot.Yaw, 0.0f));
    }
}

ELight_TimeOfDay ALightingAtmosphereManager::GetTimeOfDayFromNormalized(float NormalizedTime) const
{
    if (NormalizedTime < 0.2f || NormalizedTime >= 0.9f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.3f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.55f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.7f) return ELight_TimeOfDay::Afternoon;
    return ELight_TimeOfDay::Dusk;
}

void ALightingAtmosphereManager::InitializeDefaultPalettes()
{
    // Dawn palette — warm pink-orange, low sun, dense fog
    DawnPalette.SunPitch = -5.0f;
    DawnPalette.SunYaw = 90.0f;
    DawnPalette.SunIntensity = 3.5f;
    DawnPalette.SunColor = FLinearColor(1.0f, 0.65f, 0.35f, 1.0f);
    DawnPalette.FogDensity = 0.045f;
    DawnPalette.FogColor = FLinearColor(0.95f, 0.72f, 0.55f, 1.0f);
    DawnPalette.SkyLightIntensity = 1.2f;
    DawnPalette.SkyLightColor = FLinearColor(1.0f, 0.85f, 0.75f, 1.0f);
    DawnPalette.AutoExposureBias = -0.5f;
    DawnPalette.BloomIntensity = 0.8f;

    // Midday palette — harsh white-yellow, nearly overhead, thin haze
    MiddayPalette.SunPitch = -75.0f;
    MiddayPalette.SunYaw = 180.0f;
    MiddayPalette.SunIntensity = 12.0f;
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddayPalette.FogDensity = 0.008f;
    MiddayPalette.FogColor = FLinearColor(0.72f, 0.82f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 2.5f;
    MiddayPalette.SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MiddayPalette.AutoExposureBias = 0.3f;
    MiddayPalette.BloomIntensity = 0.4f;

    // Dusk palette — warm amber-orange, low horizon, moderate fog
    DuskPalette.SunPitch = -18.0f;
    DuskPalette.SunYaw = 85.0f;
    DuskPalette.SunIntensity = 4.5f;
    DuskPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.15f, 1.0f);
    DuskPalette.FogDensity = 0.035f;
    DuskPalette.FogColor = FLinearColor(0.85f, 0.58f, 0.35f, 1.0f);
    DuskPalette.SkyLightIntensity = 1.8f;
    DuskPalette.SkyLightColor = FLinearColor(1.0f, 0.75f, 0.55f, 1.0f);
    DuskPalette.AutoExposureBias = -0.3f;
    DuskPalette.BloomIntensity = 0.7f;

    // Night palette — very dim blue moonlight, heavy fog
    NightPalette.SunPitch = 10.0f; // Below horizon
    NightPalette.SunYaw = 0.0f;
    NightPalette.SunIntensity = 0.1f;
    NightPalette.SunColor = FLinearColor(0.3f, 0.35f, 0.6f, 1.0f);
    NightPalette.FogDensity = 0.06f;
    NightPalette.FogColor = FLinearColor(0.05f, 0.08f, 0.18f, 1.0f);
    NightPalette.SkyLightIntensity = 0.3f;
    NightPalette.SkyLightColor = FLinearColor(0.2f, 0.25f, 0.5f, 1.0f);
    NightPalette.AutoExposureBias = -1.5f;
    NightPalette.BloomIntensity = 1.2f;

    // Stormy palette — dark grey, diffuse, heavy fog
    StormyPalette.SunPitch = -45.0f;
    StormyPalette.SunYaw = 180.0f;
    StormyPalette.SunIntensity = 1.5f;
    StormyPalette.SunColor = FLinearColor(0.6f, 0.62f, 0.65f, 1.0f);
    StormyPalette.FogDensity = 0.08f;
    StormyPalette.FogColor = FLinearColor(0.45f, 0.48f, 0.52f, 1.0f);
    StormyPalette.SkyLightIntensity = 0.8f;
    StormyPalette.SkyLightColor = FLinearColor(0.55f, 0.58f, 0.65f, 1.0f);
    StormyPalette.AutoExposureBias = -0.8f;
    StormyPalette.BloomIntensity = 0.2f;
}
