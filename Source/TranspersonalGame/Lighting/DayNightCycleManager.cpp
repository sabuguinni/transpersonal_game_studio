#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Math/UnrealMathUtility.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10x per second for smooth transitions

    CurrentTimeOfDay = 6.0f; // Start at dawn
    TimeScale = 60.0f;       // 1 real second = 1 game minute
    bCycleActive = true;
    CurrentPhase = ELight_TimeOfDay::Dawn;

    SunLight = nullptr;
    SkyLightActor = nullptr;
    HeightFog = nullptr;

    InitDefaultPalettes();
}

void ADayNightCycleManager::InitDefaultPalettes()
{
    // Dawn — pink-purple horizon, soft warm light
    DawnPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.55f, 1.0f);
    DawnPalette.SunIntensity = 2.5f;
    DawnPalette.SunPitch = -8.0f;
    DawnPalette.SunYaw = -90.0f;
    DawnPalette.FogColor = FLinearColor(0.85f, 0.65f, 0.75f, 1.0f);
    DawnPalette.FogDensity = 0.045f;
    DawnPalette.SkyLightColor = FLinearColor(0.75f, 0.65f, 0.85f, 1.0f);
    DawnPalette.SkyLightIntensity = 0.8f;

    // Morning — golden warm light, clear sky
    MorningPalette.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    MorningPalette.SunIntensity = 7.0f;
    MorningPalette.SunPitch = -35.0f;
    MorningPalette.SunYaw = -60.0f;
    MorningPalette.FogColor = FLinearColor(0.75f, 0.82f, 0.95f, 1.0f);
    MorningPalette.FogDensity = 0.025f;
    MorningPalette.SkyLightColor = FLinearColor(0.85f, 0.88f, 1.0f, 1.0f);
    MorningPalette.SkyLightIntensity = 1.4f;

    // Midday — harsh white-yellow overhead sun
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddayPalette.SunIntensity = 12.0f;
    MiddayPalette.SunPitch = -75.0f;
    MiddayPalette.SunYaw = 180.0f;
    MiddayPalette.FogColor = FLinearColor(0.65f, 0.78f, 1.0f, 1.0f);
    MiddayPalette.FogDensity = 0.018f;
    MiddayPalette.SkyLightColor = FLinearColor(0.9f, 0.92f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 2.0f;

    // Dusk — deep orange-red, long shadows
    DuskPalette.SunColor = FLinearColor(1.0f, 0.62f, 0.28f, 1.0f);
    DuskPalette.SunIntensity = 4.5f;
    DuskPalette.SunPitch = -18.0f;
    DuskPalette.SunYaw = 85.0f;
    DuskPalette.FogColor = FLinearColor(0.95f, 0.55f, 0.25f, 1.0f);
    DuskPalette.FogDensity = 0.035f;
    DuskPalette.SkyLightColor = FLinearColor(0.85f, 0.65f, 0.55f, 1.0f);
    DuskPalette.SkyLightIntensity = 1.8f;

    // Night — deep blue-black, moonlight
    NightPalette.SunColor = FLinearColor(0.12f, 0.15f, 0.35f, 1.0f);
    NightPalette.SunIntensity = 0.3f;
    NightPalette.SunPitch = -5.0f;
    NightPalette.SunYaw = 270.0f;
    NightPalette.FogColor = FLinearColor(0.08f, 0.10f, 0.22f, 1.0f);
    NightPalette.FogDensity = 0.055f;
    NightPalette.SkyLightColor = FLinearColor(0.15f, 0.18f, 0.38f, 1.0f);
    NightPalette.SkyLightIntensity = 0.4f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find scene components if not assigned in editor
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

    if (!HeightFog)
    {
        TArray<AActor*> FoundFogs;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFogs);
        if (FoundFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
        }
    }

    // Apply initial palette
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive) return;

    // Advance time: TimeScale minutes per real second
    float HoursPerSecond = TimeScale / 60.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;

    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Update phase
    CurrentPhase = HourToPhase(CurrentTimeOfDay);

    // Update lighting
    UpdateLighting(CurrentTimeOfDay);
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeOfDay = FMath::Clamp(NewHour, 0.0f, 24.0f);
    CurrentPhase = HourToPhase(CurrentTimeOfDay);
    UpdateLighting(CurrentTimeOfDay);
}

ELight_TimeOfDay ADayNightCycleManager::HourToPhase(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.5f)   return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.5f && Hour < 11.0f)  return ELight_TimeOfDay::Morning;
    if (Hour >= 11.0f && Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.0f && Hour < 17.5f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.5f && Hour < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 20.0f || Hour < 2.0f)  return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

void ADayNightCycleManager::UpdateLighting(float Hour)
{
    FLight_SkyPalette CurrentPalette = GetPaletteForHour(Hour);
    ApplyPalette(CurrentPalette);
    UpdateSunPosition(Hour);
}

FLight_SkyPalette ADayNightCycleManager::GetPaletteForHour(float Hour) const
{
    // Smooth interpolation between palettes based on hour
    // Dawn: 5-7.5, Morning: 7.5-11, Midday: 11-14, Afternoon: 14-17.5, Dusk: 17.5-20, Night: 20-5

    if (Hour >= 5.0f && Hour < 7.5f)
    {
        float Alpha = (Hour - 5.0f) / 2.5f;
        return LerpPalette(NightPalette, DawnPalette, Alpha);
    }
    else if (Hour >= 7.5f && Hour < 11.0f)
    {
        float Alpha = (Hour - 7.5f) / 3.5f;
        return LerpPalette(DawnPalette, MorningPalette, Alpha);
    }
    else if (Hour >= 11.0f && Hour < 14.0f)
    {
        float Alpha = (Hour - 11.0f) / 3.0f;
        return LerpPalette(MorningPalette, MiddayPalette, Alpha);
    }
    else if (Hour >= 14.0f && Hour < 17.5f)
    {
        float Alpha = (Hour - 14.0f) / 3.5f;
        return LerpPalette(MiddayPalette, DuskPalette, Alpha);
    }
    else if (Hour >= 17.5f && Hour < 20.0f)
    {
        float Alpha = (Hour - 17.5f) / 2.5f;
        return LerpPalette(DuskPalette, NightPalette, Alpha);
    }
    else
    {
        // Night hours
        return NightPalette;
    }
}

FLight_SkyPalette ADayNightCycleManager::LerpPalette(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha)
{
    FLight_SkyPalette Result;
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightColor = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, Alpha),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, Alpha),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::UpdateSunPosition(float Hour)
{
    if (!SunLight) return;

    // Sun arc: rises in east (-90 yaw), sets in west (90 yaw)
    // Pitch: -90 at noon (overhead), near 0 at horizon
    float NormalizedHour = (Hour - 6.0f) / 12.0f; // 0 at 6am, 1 at 6pm
    NormalizedHour = FMath::Clamp(NormalizedHour, 0.0f, 1.0f);

    float SunYaw = FMath::Lerp(-90.0f, 90.0f, NormalizedHour);
    float SunPitch = -FMath::Sin(NormalizedHour * PI) * 75.0f; // Max -75° at noon

    // Night: moon position
    if (Hour < 6.0f || Hour > 18.0f)
    {
        float NightNorm = (Hour < 6.0f) ? (Hour + 6.0f) / 12.0f : (Hour - 18.0f) / 12.0f;
        SunYaw = FMath::Lerp(90.0f, 270.0f, NightNorm);
        SunPitch = -FMath::Sin(NightNorm * PI) * 45.0f;
    }

    SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.0f));
}

void ADayNightCycleManager::ApplyPalette(const FLight_SkyPalette& Palette)
{
    // Apply to directional light
    if (SunLight)
    {
        UDirectionalLightComponent* DirComp = SunLight->GetComponent();
        if (DirComp)
        {
            DirComp->SetIntensity(Palette.SunIntensity);
            DirComp->SetLightColor(Palette.SunColor);
        }
    }

    // Apply to sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }

    // Apply to height fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }
}
