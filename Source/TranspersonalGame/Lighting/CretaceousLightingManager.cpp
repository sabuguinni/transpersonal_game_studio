#include "CretaceousLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Tick every 0.5s for performance

    InitDefaultConfigs();
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lighting actors in world if not assigned
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
        TArray<AActor*> FoundFogs;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFogs);
        if (FoundFogs.Num() > 0)
        {
            HeightFogActor = Cast<AExponentialHeightFog>(FoundFogs[0]);
        }
    }

    CurrentHour = StartHour;
    SetTimeOfDay(HourToTimeOfDay(CurrentHour));
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        AdvanceDayNightCycle(DeltaTime);
    }
}

void ACretaceousLightingManager::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;

    FLight_TimeOfDayConfig ConfigToApply;

    switch (NewTime)
    {
        case ELight_TimeOfDay::Dawn:
        {
            FLight_TimeOfDayConfig DawnCfg;
            DawnCfg.SunPitch = -8.0f;
            DawnCfg.SunIntensity = 4.0f;
            DawnCfg.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            DawnCfg.SkyLightIntensity = 0.8f;
            DawnCfg.FogDensity = 0.08f;
            DawnCfg.FogColor = FLinearColor(0.6f, 0.5f, 0.7f, 1.0f);
            ConfigToApply = DawnCfg;
            break;
        }
        case ELight_TimeOfDay::Morning:
        {
            FLight_TimeOfDayConfig MornCfg;
            MornCfg.SunPitch = -25.0f;
            MornCfg.SunIntensity = 8.0f;
            MornCfg.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            MornCfg.SkyLightIntensity = 1.5f;
            MornCfg.FogDensity = 0.05f;
            MornCfg.FogColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
            ConfigToApply = MornCfg;
            break;
        }
        case ELight_TimeOfDay::Midday:
            ConfigToApply = MiddayConfig;
            break;
        case ELight_TimeOfDay::GoldenHour:
            ConfigToApply = GoldenHourConfig;
            break;
        case ELight_TimeOfDay::Dusk:
        {
            FLight_TimeOfDayConfig DuskCfg;
            DuskCfg.SunPitch = -5.0f;
            DuskCfg.SunIntensity = 3.0f;
            DuskCfg.SunColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
            DuskCfg.SkyLightIntensity = 0.6f;
            DuskCfg.FogDensity = 0.06f;
            DuskCfg.FogColor = FLinearColor(0.7f, 0.4f, 0.3f, 1.0f);
            ConfigToApply = DuskCfg;
            break;
        }
        case ELight_TimeOfDay::Night:
            ConfigToApply = NightConfig;
            break;
        default:
            ConfigToApply = GoldenHourConfig;
            break;
    }

    ApplyLightingConfig(ConfigToApply);
}

void ACretaceousLightingManager::ApplyLightingConfig(const FLight_TimeOfDayConfig& Config)
{
    // Apply to DirectionalLight (Sun)
    if (SunLight)
    {
        FRotator SunRot = SunLight->GetActorRotation();
        SunRot.Pitch = Config.SunPitch;
        SunLight->SetActorRotation(SunRot);

        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Config.SunIntensity);
            DLC->SetLightColor(Config.SunColor.ToFColor(true));
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Config.SkyLightIntensity);
        }
    }

    // Apply to Fog
    if (HeightFogActor)
    {
        UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Config.FogDensity);
            FogComp->SetFogInscatteringColor(Config.FogColor.ToFColor(true));
        }
    }
}

void ACretaceousLightingManager::AdvanceDayNightCycle(float DeltaSeconds)
{
    // Advance time: full 24h cycle over DayDurationSeconds
    float HoursPerSecond = 24.0f / FMath::Max(DayDurationSeconds, 1.0f);
    CurrentHour += HoursPerSecond * DeltaSeconds;

    if (CurrentHour >= 24.0f)
    {
        CurrentHour -= 24.0f;
    }

    ELight_TimeOfDay NewTOD = HourToTimeOfDay(CurrentHour);
    if (NewTOD != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTOD);
    }
    else
    {
        // Smooth interpolation within the same period
        // Recalculate sun pitch based on exact hour (0-24 maps to full rotation)
        if (SunLight)
        {
            // Sun arc: rises at hour 6 (pitch 0), peaks at hour 12 (pitch -90), sets at hour 18
            float SunAngle = (CurrentHour - 6.0f) / 12.0f * 180.0f; // 0 to 180 degrees
            float SunPitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;
            FRotator SunRot = SunLight->GetActorRotation();
            SunRot.Pitch = SunPitch;
            SunLight->SetActorRotation(SunRot);
        }
    }
}

void ACretaceousLightingManager::InitDefaultConfigs()
{
    // Golden Hour (default — cinematic Cretaceous look)
    GoldenHourConfig.SunPitch = -38.0f;
    GoldenHourConfig.SunIntensity = 12.0f;
    GoldenHourConfig.SunColor = FLinearColor(1.0f, 0.878f, 0.619f, 1.0f);
    GoldenHourConfig.SkyLightIntensity = 2.0f;
    GoldenHourConfig.FogDensity = 0.04f;
    GoldenHourConfig.FogColor = FLinearColor(0.45f, 0.65f, 0.85f, 1.0f);

    // Midday (harsh, bright)
    MiddayConfig.SunPitch = -80.0f;
    MiddayConfig.SunIntensity = 15.0f;
    MiddayConfig.SunColor = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    MiddayConfig.SkyLightIntensity = 3.0f;
    MiddayConfig.FogDensity = 0.02f;
    MiddayConfig.FogColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    // Night (dark, moonlit)
    NightConfig.SunPitch = 30.0f; // Below horizon
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightConfig.SkyLightIntensity = 0.3f;
    NightConfig.FogDensity = 0.07f;
    NightConfig.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
}

ELight_TimeOfDay ACretaceousLightingManager::HourToTimeOfDay(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 15.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 15.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.0f && Hour < 19.5f) return ELight_TimeOfDay::GoldenHour;
    if (Hour >= 19.5f && Hour < 21.0f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}

FLight_TimeOfDayConfig ACretaceousLightingManager::LerpConfigs(
    const FLight_TimeOfDayConfig& A,
    const FLight_TimeOfDayConfig& B,
    float Alpha) const
{
    FLight_TimeOfDayConfig Result;
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    return Result;
}
