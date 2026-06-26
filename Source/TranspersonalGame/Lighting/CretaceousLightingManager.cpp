#include "CretaceousLightingManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth day/night

    // Default: Cretaceous golden hour
    SunConfig.Pitch = -38.0f;
    SunConfig.Intensity = 12.0f;
    SunConfig.LightColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);
    SunConfig.bAtmosphereSunLight = true;

    FogConfig.FogDensity = 0.035f;
    FogConfig.HeightFalloff = 0.2f;
    FogConfig.FogColor = FLinearColor(0.45f, 0.62f, 0.78f, 1.0f);
    FogConfig.bVolumetricFog = true;
    FogConfig.VolumetricFogExtinctionScale = 1.2f;

    SkyLightIntensity = 2.5f;
    DayDurationSeconds = 1200.0f;
    TimeOfDaySeconds = 300.0f; // Start at golden hour (25% through day)
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find sun and skylight in level if not assigned
    if (!SunActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunActor = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }

    // Apply initial lighting state
    ApplySunConfig(SunConfig);
    ApplyFogConfig(FogConfig);
    ApplyLumenSettings();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicDayNight)
    {
        AdvanceDayNightCycle(DeltaTime);
    }
}

void ACretaceousLightingManager::AdvanceDayNightCycle(float DeltaTime)
{
    TimeOfDaySeconds += DeltaTime;
    if (TimeOfDaySeconds >= DayDurationSeconds)
    {
        TimeOfDaySeconds = 0.0f;
    }

    const float NormalizedTime = TimeOfDaySeconds / DayDurationSeconds;

    // Update sun pitch and color
    const float NewPitch = ComputeSunPitchFromTime(NormalizedTime);
    const FLinearColor NewColor = ComputeSunColorFromTime(NormalizedTime);

    UpdateSunTransform(NewPitch);

    if (SunActor)
    {
        UDirectionalLightComponent* DLC = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetLightColor(NewColor);

            // Intensity varies with time: brightest at noon, dim at dawn/dusk, zero at night
            const float IntensityMultiplier = FMath::Clamp(FMath::Sin(NormalizedTime * PI), 0.0f, 1.0f);
            DLC->SetIntensity(SunConfig.Intensity * IntensityMultiplier);
        }
    }

    // Determine time of day enum
    if (NormalizedTime < 0.1f)       CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (NormalizedTime < 0.3f)  CurrentTimeOfDay = ELight_TimeOfDay::Morning;
    else if (NormalizedTime < 0.55f) CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    else if (NormalizedTime < 0.7f)  CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
    else if (NormalizedTime < 0.8f)  CurrentTimeOfDay = ELight_TimeOfDay::Dusk;
    else                             CurrentTimeOfDay = ELight_TimeOfDay::Night;

    // Recapture skylight periodically (every 5 ticks = 0.5s)
    static int32 TickCount = 0;
    TickCount++;
    if (TickCount % 5 == 0)
    {
        UpdateSkyLightCapture();
    }
}

float ACretaceousLightingManager::ComputeSunPitchFromTime(float NormalizedTime) const
{
    // Sun arc: rises from -90 (below horizon) at dawn, peaks at -10 (high noon), sets at -90 at dusk
    // NormalizedTime: 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1=midnight
    const float SunAngle = (NormalizedTime - 0.25f) * 2.0f * PI;
    const float Pitch = FMath::Sin(SunAngle) * 80.0f - 10.0f;
    return FMath::Clamp(Pitch, -90.0f, -5.0f);
}

FLinearColor ACretaceousLightingManager::ComputeSunColorFromTime(float NormalizedTime) const
{
    // Dawn: orange-pink, Midday: white-yellow, Dusk: deep orange-red, Night: blue
    if (NormalizedTime < 0.1f)
    {
        // Pre-dawn: deep blue
        return FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    }
    else if (NormalizedTime < 0.2f)
    {
        // Dawn: orange-pink
        return FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    }
    else if (NormalizedTime < 0.45f)
    {
        // Morning: warm white
        return FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    }
    else if (NormalizedTime < 0.55f)
    {
        // Midday: bright white-yellow
        return FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    }
    else if (NormalizedTime < 0.7f)
    {
        // Golden hour: warm gold
        return FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);
    }
    else if (NormalizedTime < 0.8f)
    {
        // Dusk: deep orange-red
        return FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    }
    else
    {
        // Night: moonlight blue
        return FLinearColor(0.15f, 0.2f, 0.45f, 1.0f);
    }
}

void ACretaceousLightingManager::UpdateSunTransform(float Pitch)
{
    if (SunActor)
    {
        FRotator CurrentRot = SunActor->GetActorRotation();
        SunActor->SetActorRotation(FRotator(Pitch, CurrentRot.Yaw, 0.0f));
    }
}

void ACretaceousLightingManager::UpdateSkyLightCapture()
{
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC && SLC->IsRealTimeCaptureEnabled())
        {
            SLC->RecaptureSky();
        }
    }
}

void ACretaceousLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    switch (NewTime)
    {
    case ELight_TimeOfDay::Dawn:
        TimeOfDaySeconds = DayDurationSeconds * 0.15f;
        break;
    case ELight_TimeOfDay::Morning:
        TimeOfDaySeconds = DayDurationSeconds * 0.25f;
        break;
    case ELight_TimeOfDay::Midday:
        TimeOfDaySeconds = DayDurationSeconds * 0.5f;
        break;
    case ELight_TimeOfDay::GoldenHour:
        TimeOfDaySeconds = DayDurationSeconds * 0.65f;
        break;
    case ELight_TimeOfDay::Dusk:
        TimeOfDaySeconds = DayDurationSeconds * 0.75f;
        break;
    case ELight_TimeOfDay::Night:
        TimeOfDaySeconds = DayDurationSeconds * 0.9f;
        break;
    }
}

void ACretaceousLightingManager::ApplySunConfig(const FLight_SunConfig& Config)
{
    SunConfig = Config;

    if (SunActor)
    {
        SunActor->SetActorRotation(FRotator(Config.Pitch, 45.0f, 0.0f));
        UDirectionalLightComponent* DLC = SunActor->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Config.Intensity);
            DLC->SetLightColor(Config.LightColor);
            DLC->SetAtmosphereSunLight(Config.bAtmosphereSunLight);
        }
    }
}

void ACretaceousLightingManager::ApplyFogConfig(const FLight_FogConfig& Config)
{
    FogConfig = Config;

    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);

    if (FogActors.Num() > 0)
    {
        AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        if (FogActor)
        {
            UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
            if (FogComp)
            {
                FogComp->SetFogDensity(Config.FogDensity);
                FogComp->SetFogHeightFalloff(Config.HeightFalloff);
                FogComp->SetFogInscatteringColor(Config.FogColor);
                FogComp->SetVolumetricFog(Config.bVolumetricFog);
            }
        }
    }
}

void ACretaceousLightingManager::ApplyLumenSettings()
{
    if (!GetWorld()) return;

    UWorld* World = GetWorld();

    // Apply Lumen console variables
    auto ExecCmd = [&](const FString& Cmd)
    {
        GEngine->Exec(World, *Cmd);
    };

    if (bEnableLumenGI)
    {
        ExecCmd(TEXT("r.Lumen.GlobalIllumination.Allow 1"));
        ExecCmd(TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
        ExecCmd(TEXT("r.Lumen.TraceMeshSDFs 1"));
    }

    if (bEnableLumenReflections)
    {
        ExecCmd(TEXT("r.Lumen.Reflections.Allow 1"));
        ExecCmd(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace 0.4"));
    }

    if (bEnableVolumetricFog)
    {
        ExecCmd(TEXT("r.VolumetricFog 1"));
        ExecCmd(TEXT("r.VolumetricFog.GridPixelSize 8"));
        ExecCmd(TEXT("r.VolumetricFog.GridSizeZ 64"));
    }

    ExecCmd(TEXT("r.FastSkyLUT 0"));
    ExecCmd(TEXT("r.SkyAtmosphere.FastSkyLUT 0"));
    ExecCmd(TEXT("r.SkyLight.RealTimeReflectionCapture 1"));
    ExecCmd(TEXT("r.Shadow.DistanceScale 2.0"));
}

float ACretaceousLightingManager::GetCurrentSunPitch() const
{
    if (SunActor)
    {
        return SunActor->GetActorRotation().Pitch;
    }
    return SunConfig.Pitch;
}

FLinearColor ACretaceousLightingManager::GetCurrentSkyColor() const
{
    const float NormalizedTime = TimeOfDaySeconds / DayDurationSeconds;
    return ComputeSunColorFromTime(NormalizedTime);
}

void ACretaceousLightingManager::ApplyGoldenHourPreset()
{
    FLight_SunConfig GoldenConfig;
    GoldenConfig.Pitch = -38.0f;
    GoldenConfig.Intensity = 12.0f;
    GoldenConfig.LightColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);
    GoldenConfig.bAtmosphereSunLight = true;
    ApplySunConfig(GoldenConfig);

    FLight_FogConfig GoldenFog;
    GoldenFog.FogDensity = 0.035f;
    GoldenFog.FogColor = FLinearColor(0.55f, 0.45f, 0.35f, 1.0f);
    GoldenFog.bVolumetricFog = true;
    ApplyFogConfig(GoldenFog);

    TimeOfDaySeconds = DayDurationSeconds * 0.65f;
    CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;
}

void ACretaceousLightingManager::ApplyMidnightPreset()
{
    FLight_SunConfig NightConfig;
    NightConfig.Pitch = -90.0f;
    NightConfig.Intensity = 0.5f;
    NightConfig.LightColor = FLinearColor(0.15f, 0.2f, 0.45f, 1.0f);
    NightConfig.bAtmosphereSunLight = true;
    ApplySunConfig(NightConfig);

    FLight_FogConfig NightFog;
    NightFog.FogDensity = 0.05f;
    NightFog.FogColor = FLinearColor(0.05f, 0.07f, 0.15f, 1.0f);
    NightFog.bVolumetricFog = true;
    ApplyFogConfig(NightFog);

    TimeOfDaySeconds = DayDurationSeconds * 0.9f;
    CurrentTimeOfDay = ELight_TimeOfDay::Night;
}

void ACretaceousLightingManager::ApplyDawnPreset()
{
    FLight_SunConfig DawnConfig;
    DawnConfig.Pitch = -75.0f;
    DawnConfig.Intensity = 4.0f;
    DawnConfig.LightColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DawnConfig.bAtmosphereSunLight = true;
    ApplySunConfig(DawnConfig);

    FLight_FogConfig DawnFog;
    DawnFog.FogDensity = 0.06f;
    DawnFog.FogColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    DawnFog.bVolumetricFog = true;
    ApplyFogConfig(DawnFog);

    TimeOfDaySeconds = DayDurationSeconds * 0.15f;
    CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
}
