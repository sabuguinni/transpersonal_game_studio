#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for performance

    // Default palette presets
    // Dawn
    DawnPalette.SunPitch = -8.0f;
    DawnPalette.SunYaw = -90.0f;
    DawnPalette.SunIntensity = 2.5f;
    DawnPalette.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnPalette.FogDensity = 0.045f;
    DawnPalette.FogColor = FLinearColor(0.8f, 0.55f, 0.35f, 1.0f);
    DawnPalette.SkyLightIntensity = 1.2f;
    DawnPalette.SkyLightColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);

    // Midday
    MiddayPalette.SunPitch = -75.0f;
    MiddayPalette.SunYaw = 0.0f;
    MiddayPalette.SunIntensity = 12.0f;
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddayPalette.FogDensity = 0.008f;
    MiddayPalette.FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 2.5f;
    MiddayPalette.SkyLightColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    // Dusk
    DuskPalette.SunPitch = -18.0f;
    DuskPalette.SunYaw = 85.0f;
    DuskPalette.SunIntensity = 4.5f;
    DuskPalette.SunColor = FLinearColor(1.0f, 0.45f, 0.1f, 1.0f);
    DuskPalette.FogDensity = 0.035f;
    DuskPalette.FogColor = FLinearColor(0.7f, 0.35f, 0.15f, 1.0f);
    DuskPalette.SkyLightIntensity = 1.8f;
    DuskPalette.SkyLightColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    // Night
    NightPalette.SunPitch = -15.0f;
    NightPalette.SunYaw = 200.0f;
    NightPalette.SunIntensity = 0.8f;
    NightPalette.SunColor = FLinearColor(0.78f, 0.82f, 1.0f, 1.0f);
    NightPalette.FogDensity = 0.055f;
    NightPalette.FogColor = FLinearColor(0.05f, 0.06f, 0.15f, 1.0f);
    NightPalette.SkyLightIntensity = 0.4f;
    NightPalette.SkyLightColor = FLinearColor(0.7f, 0.74f, 1.0f, 1.0f);
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    SetTimeOfDay(TimeOfDayNormalized);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bEnableDynamicDayNight)
    {
        UpdateLighting(DeltaTime);
    }
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance time
    const float DayDurationSeconds = 86400.0f / DayNightSpeedMultiplier;
    ElapsedDayTime += DeltaTime;
    if (ElapsedDayTime >= DayDurationSeconds)
    {
        ElapsedDayTime -= DayDurationSeconds;
    }
    TimeOfDayNormalized = ElapsedDayTime / DayDurationSeconds;
    SetTimeOfDay(TimeOfDayNormalized);
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);

    // Determine current time of day enum
    if (NormalizedTime < 0.1f || NormalizedTime >= 0.95f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midnight;
    else if (NormalizedTime < 0.2f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (NormalizedTime < 0.35f)
        CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (NormalizedTime < 0.55f)
        CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (NormalizedTime < 0.7f)
        CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (NormalizedTime < 0.8f)
        CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = ELight_TimeOfDay::Night;

    // Interpolate between palettes based on time
    FLight_TimeOfDayPalette TargetPalette;

    if (NormalizedTime < 0.2f)
    {
        // Midnight → Dawn
        float Alpha = NormalizedTime / 0.2f;
        TargetPalette = InterpolatePalettes(NightPalette, DawnPalette, Alpha);
    }
    else if (NormalizedTime < 0.45f)
    {
        // Dawn → Midday
        float Alpha = (NormalizedTime - 0.2f) / 0.25f;
        TargetPalette = InterpolatePalettes(DawnPalette, MiddayPalette, Alpha);
    }
    else if (NormalizedTime < 0.7f)
    {
        // Midday → Dusk
        float Alpha = (NormalizedTime - 0.45f) / 0.25f;
        TargetPalette = InterpolatePalettes(MiddayPalette, DuskPalette, Alpha);
    }
    else
    {
        // Dusk → Night
        float Alpha = (NormalizedTime - 0.7f) / 0.3f;
        TargetPalette = InterpolatePalettes(DuskPalette, NightPalette, Alpha);
    }

    ApplyPalette(TargetPalette);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

void ADayNightCycleManager::ApplyPalette(const FLight_TimeOfDayPalette& Palette)
{
    if (!SunLight) return;

    // Apply sun rotation
    FRotator SunRot(Palette.SunPitch, Palette.SunYaw, 0.0f);
    SunLight->SetActorRotation(SunRot);

    UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
    if (DLC)
    {
        DLC->SetIntensity(Palette.SunIntensity);
        DLC->SetLightColor(Palette.SunColor);
    }

    // Apply fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply skylight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Palette.SkyLightIntensity);
            SLC->SetLightColor(Palette.SkyLightColor);
        }
    }
}

void ADayNightCycleManager::SnapToNight()
{
    SetTimeOfDay(0.85f);
}

void ADayNightCycleManager::SnapToDawn()
{
    SetTimeOfDay(0.15f);
}

void ADayNightCycleManager::SnapToMidday()
{
    SetTimeOfDay(0.5f);
}

FLight_TimeOfDayPalette ADayNightCycleManager::InterpolatePalettes(
    const FLight_TimeOfDayPalette& A,
    const FLight_TimeOfDayPalette& B,
    float Alpha) const
{
    FLight_TimeOfDayPalette Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
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
    return Result;
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find DirectionalLight if not set
    if (!SunLight)
    {
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
    }

    // Auto-find ExponentialHeightFog
    if (!HeightFog)
    {
        TArray<AActor*> FoundFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
        if (FoundFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
        }
    }

    // Auto-find SkyLight
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundSkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundSkyLights);
        if (FoundSkyLights.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundSkyLights[0]);
        }
    }
}
