#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x/sec for smooth cycle

    // ─── Golden Hour defaults (Cretaceous afternoon) ───────────────────────────
    GoldenHourConfig.SunPitch = -38.0f;
    GoldenHourConfig.SunYaw = 45.0f;
    GoldenHourConfig.SunIntensity = 12.0f;
    GoldenHourConfig.SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);
    GoldenHourConfig.FogDensity = 0.018f;
    GoldenHourConfig.FogColor = FLinearColor(0.45f, 0.62f, 0.85f, 1.0f);
    GoldenHourConfig.SkyLightIntensity = 2.0f;
    GoldenHourConfig.ExposureBias = 0.3f;

    // ─── Midday defaults (harsh overhead sun) ─────────────────────────────────
    MiddayConfig.SunPitch = -75.0f;
    MiddayConfig.SunYaw = 0.0f;
    MiddayConfig.SunIntensity = 18.0f;
    MiddayConfig.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddayConfig.FogDensity = 0.008f;
    MiddayConfig.FogColor = FLinearColor(0.6f, 0.75f, 0.95f, 1.0f);
    MiddayConfig.SkyLightIntensity = 3.0f;
    MiddayConfig.ExposureBias = 0.0f;

    // ─── Dawn defaults (cool purple-pink) ─────────────────────────────────────
    DawnConfig.SunPitch = -5.0f;
    DawnConfig.SunYaw = -90.0f;
    DawnConfig.SunIntensity = 4.0f;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.7f, 0.55f, 1.0f);
    DawnConfig.FogDensity = 0.035f;
    DawnConfig.FogColor = FLinearColor(0.55f, 0.5f, 0.75f, 1.0f);
    DawnConfig.SkyLightIntensity = 0.8f;
    DawnConfig.ExposureBias = -0.5f;

    // ─── Night defaults (deep blue, moonlit) ──────────────────────────────────
    NightConfig.SunPitch = 30.0f; // Below horizon
    NightConfig.SunYaw = 180.0f;
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightConfig.FogDensity = 0.025f;
    NightConfig.FogColor = FLinearColor(0.1f, 0.15f, 0.35f, 1.0f);
    NightConfig.SkyLightIntensity = 0.3f;
    NightConfig.ExposureBias = -1.5f;
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    SetTimeOfDay(TimeOfDayNormalized);
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayCycleActive && DayDurationSeconds > 0.0f)
    {
        float DeltaNormalized = DeltaTime / DayDurationSeconds;
        TimeOfDayNormalized = FMath::Fmod(TimeOfDayNormalized + DeltaNormalized, 1.0f);
        SetTimeOfDay(TimeOfDayNormalized);
    }
}

void ACretaceousLightingManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentTimeOfDay = ClassifyTimeOfDay(TimeOfDayNormalized);

    // Determine which configs to blend between
    FLight_TimeOfDayConfig BlendedConfig;

    if (NormalizedTime < 0.25f) // Night → Dawn
    {
        float Alpha = NormalizedTime / 0.25f;
        BlendedConfig = LerpLightingConfig(NightConfig, DawnConfig, Alpha);
    }
    else if (NormalizedTime < 0.42f) // Dawn → Midday
    {
        float Alpha = (NormalizedTime - 0.25f) / 0.17f;
        BlendedConfig = LerpLightingConfig(DawnConfig, MiddayConfig, Alpha);
    }
    else if (NormalizedTime < 0.58f) // Midday → Golden Hour
    {
        float Alpha = (NormalizedTime - 0.42f) / 0.16f;
        BlendedConfig = LerpLightingConfig(MiddayConfig, GoldenHourConfig, Alpha);
    }
    else if (NormalizedTime < 0.70f) // Golden Hour → Dusk
    {
        float Alpha = (NormalizedTime - 0.58f) / 0.12f;
        FLight_TimeOfDayConfig DuskConfig = GoldenHourConfig;
        DuskConfig.SunPitch = -8.0f;
        DuskConfig.SunIntensity = 5.0f;
        DuskConfig.SunColor = FLinearColor(1.0f, 0.55f, 0.3f, 1.0f);
        DuskConfig.FogDensity = 0.028f;
        BlendedConfig = LerpLightingConfig(GoldenHourConfig, DuskConfig, Alpha);
    }
    else // Dusk → Night
    {
        float Alpha = (NormalizedTime - 0.70f) / 0.30f;
        FLight_TimeOfDayConfig DuskConfig = GoldenHourConfig;
        DuskConfig.SunPitch = -8.0f;
        DuskConfig.SunIntensity = 5.0f;
        DuskConfig.SunColor = FLinearColor(1.0f, 0.55f, 0.3f, 1.0f);
        BlendedConfig = LerpLightingConfig(DuskConfig, NightConfig, Alpha);
    }

    ApplyLightingConfig(BlendedConfig);
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    WeatherBlendAlpha = 0.0f;
}

void ACretaceousLightingManager::ApplyLightingConfig(const FLight_TimeOfDayConfig& Config)
{
    if (!GetWorld()) return;

    // ─── Apply Sun ─────────────────────────────────────────────────────────────
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Config.SunPitch, Config.SunYaw, 0.0f));
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Config.SunIntensity);
            SunComp->SetLightColor(Config.SunColor);
        }
    }

    // ─── Apply SkyLight ────────────────────────────────────────────────────────
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Config.SkyLightIntensity);
        }
    }

    // ─── Apply Fog ─────────────────────────────────────────────────────────────
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FogActors);
    for (AActor* FogActor : FogActors)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Config.FogDensity);
            FogComp->SetFogInscatteringColor(Config.FogColor);
        }
    }
}

FString ACretaceousLightingManager::GetTimeOfDayString() const
{
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:        return TEXT("Dawn");
        case ELight_TimeOfDay::Morning:     return TEXT("Morning");
        case ELight_TimeOfDay::Midday:      return TEXT("Midday");
        case ELight_TimeOfDay::Afternoon:   return TEXT("Afternoon");
        case ELight_TimeOfDay::GoldenHour:  return TEXT("Golden Hour");
        case ELight_TimeOfDay::Dusk:        return TEXT("Dusk");
        case ELight_TimeOfDay::Night:       return TEXT("Night");
        default:                            return TEXT("Unknown");
    }
}

void ACretaceousLightingManager::AutoFindLightingActors()
{
    if (!GetWorld()) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), AllActors);
    if (AllActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(AllActors[0]);
    }

    AllActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), AllActors);
    if (AllActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(AllActors[0]);
    }
}

FRotator ACretaceousLightingManager::ComputeSunRotation(float NormalizedTime) const
{
    // Sun arc: rises in east (-90 yaw), sets in west (90 yaw)
    float SunAngle = NormalizedTime * 360.0f - 90.0f;
    float Pitch = -FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 80.0f;
    float Yaw = FMath::Cos(FMath::DegreesToRadians(SunAngle)) * 45.0f;
    return FRotator(Pitch, Yaw, 0.0f);
}

FLight_TimeOfDayConfig ACretaceousLightingManager::LerpLightingConfig(
    const FLight_TimeOfDayConfig& A,
    const FLight_TimeOfDayConfig& B,
    float Alpha) const
{
    FLight_TimeOfDayConfig Result;
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
    Result.ExposureBias = FMath::Lerp(A.ExposureBias, B.ExposureBias, Alpha);
    return Result;
}

ELight_TimeOfDay ACretaceousLightingManager::ClassifyTimeOfDay(float NormalizedTime) const
{
    if (NormalizedTime < 0.20f) return ELight_TimeOfDay::Night;
    if (NormalizedTime < 0.30f) return ELight_TimeOfDay::Dawn;
    if (NormalizedTime < 0.40f) return ELight_TimeOfDay::Morning;
    if (NormalizedTime < 0.50f) return ELight_TimeOfDay::Midday;
    if (NormalizedTime < 0.60f) return ELight_TimeOfDay::Afternoon;
    if (NormalizedTime < 0.70f) return ELight_TimeOfDay::GoldenHour;
    if (NormalizedTime < 0.80f) return ELight_TimeOfDay::Dusk;
    return ELight_TimeOfDay::Night;
}
