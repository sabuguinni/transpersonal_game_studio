#include "DawnAtmosphereSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ADawnAtmosphereSystem::ADawnAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;  // Update 10x/sec for smooth transitions

    InitDefaultPalettes();
}

void ADawnAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find lighting actors if not set in editor
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

    if (!FogActor)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }

    // Apply initial palette
    ApplyPalette(GetPaletteForTime(CurrentTimeOfDay));
}

void ADawnAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        AdvanceDayCycle(DeltaTime);
    }
}

void ADawnAtmosphereSystem::AdvanceDayCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    ElapsedDayTime += DeltaTime;
    CurrentTimeNormalized = FMath::Fmod(ElapsedDayTime / DayDurationSeconds, 1.0f);

    // Determine time of day from normalized time
    // 0.0-0.15 = Night, 0.15-0.25 = Dawn, 0.25-0.45 = Morning,
    // 0.45-0.55 = Midday, 0.55-0.70 = Afternoon, 0.70-0.85 = Dusk, 0.85-1.0 = Night
    ELight_TimeOfDay NewTime = CurrentTimeOfDay;
    float T = CurrentTimeNormalized;

    if (T < 0.15f || T >= 0.85f)       NewTime = ELight_TimeOfDay::Night;
    else if (T < 0.25f)                 NewTime = ELight_TimeOfDay::Dawn;
    else if (T < 0.45f)                 NewTime = ELight_TimeOfDay::Morning;
    else if (T < 0.55f)                 NewTime = ELight_TimeOfDay::Midday;
    else if (T < 0.70f)                 NewTime = ELight_TimeOfDay::Afternoon;
    else                                NewTime = ELight_TimeOfDay::Dusk;

    // Interpolate between adjacent palettes for smooth transitions
    FLight_AtmospherePalette CurrentPalette = GetPaletteForTime(NewTime);
    ApplyPalette(CurrentPalette);

    CurrentTimeOfDay = NewTime;
}

void ADawnAtmosphereSystem::SetTimeOfDay(ELight_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    ApplyPalette(GetPaletteForTime(NewTime));
}

void ADawnAtmosphereSystem::SetTimeNormalized(float NormalizedTime)
{
    CurrentTimeNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ElapsedDayTime = CurrentTimeNormalized * DayDurationSeconds;
}

FLight_AtmospherePalette ADawnAtmosphereSystem::GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:        return DawnPalette;
        case ELight_TimeOfDay::Morning:     return MorningPalette;
        case ELight_TimeOfDay::Midday:      return MiddayPalette;
        case ELight_TimeOfDay::Afternoon:   return AfternoonPalette;
        case ELight_TimeOfDay::Dusk:        return DuskPalette;
        case ELight_TimeOfDay::Night:       return NightPalette;
        default:                            return DawnPalette;
    }
}

void ADawnAtmosphereSystem::ApplyPalette(const FLight_AtmospherePalette& Palette)
{
    // Apply to Directional Light (Sun)
    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponent();
        if (SunComp)
        {
            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
        // Guard: never set pitch above -5 degrees (sun too low causes artifacts)
        float SafePitch = FMath::Clamp(Palette.SunPitchDegrees, -89.0f, -5.0f);
        SunLight->SetActorRotation(FRotator(SafePitch, Palette.SunYawDegrees, 0.0f));
    }

    // Apply to Sky Light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }

    // Apply to Fog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
            FogComp->SetVolumetricFog(Palette.bVolumetricFog);
        }
    }
}

void ADawnAtmosphereSystem::ApplyDawnPaletteNow()
{
    SetTimeOfDay(ELight_TimeOfDay::Dawn);
}

void ADawnAtmosphereSystem::ApplyMiddayPaletteNow()
{
    SetTimeOfDay(ELight_TimeOfDay::Midday);
}

void ADawnAtmosphereSystem::ApplyDuskPaletteNow()
{
    SetTimeOfDay(ELight_TimeOfDay::Dusk);
}

FLight_AtmospherePalette ADawnAtmosphereSystem::InterpolatePalettes(
    const FLight_AtmospherePalette& A,
    const FLight_AtmospherePalette& B,
    float Alpha) const
{
    FLight_AtmospherePalette Result;
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SkyLightColor = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, Alpha),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, Alpha),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, Alpha),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogColor = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f
    );
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.bVolumetricFog = A.bVolumetricFog;
    return Result;
}

void ADawnAtmosphereSystem::InitDefaultPalettes()
{
    // DAWN — Rose-gold horizon, cool purple sky, dense mist
    DawnPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.52f, 1.0f);
    DawnPalette.SunIntensity = 3.5f;
    DawnPalette.SunPitchDegrees = -12.0f;
    DawnPalette.SunYawDegrees = 90.0f;   // East
    DawnPalette.SkyLightColor = FLinearColor(0.85f, 0.78f, 1.0f, 1.0f);
    DawnPalette.SkyLightIntensity = 1.4f;
    DawnPalette.FogColor = FLinearColor(0.9f, 0.65f, 0.55f, 1.0f);
    DawnPalette.FogDensity = 0.05f;
    DawnPalette.bVolumetricFog = true;

    // MORNING — Warm yellow, clear sky, light mist
    MorningPalette.SunColor = FLinearColor(1.0f, 0.92f, 0.7f, 1.0f);
    MorningPalette.SunIntensity = 7.0f;
    MorningPalette.SunPitchDegrees = -35.0f;
    MorningPalette.SunYawDegrees = 120.0f;
    MorningPalette.SkyLightColor = FLinearColor(0.9f, 0.92f, 1.0f, 1.0f);
    MorningPalette.SkyLightIntensity = 1.8f;
    MorningPalette.FogColor = FLinearColor(0.85f, 0.88f, 0.95f, 1.0f);
    MorningPalette.FogDensity = 0.025f;
    MorningPalette.bVolumetricFog = true;

    // MIDDAY — White-yellow, harsh, minimal fog
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    MiddayPalette.SunIntensity = 12.0f;
    MiddayPalette.SunPitchDegrees = -72.0f;
    MiddayPalette.SunYawDegrees = 180.0f;
    MiddayPalette.SkyLightColor = FLinearColor(0.95f, 0.97f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 2.2f;
    MiddayPalette.FogColor = FLinearColor(0.8f, 0.88f, 1.0f, 1.0f);
    MiddayPalette.FogDensity = 0.015f;
    MiddayPalette.bVolumetricFog = false;

    // AFTERNOON — Golden, warm shadows
    AfternoonPalette.SunColor = FLinearColor(1.0f, 0.85f, 0.55f, 1.0f);
    AfternoonPalette.SunIntensity = 8.5f;
    AfternoonPalette.SunPitchDegrees = -35.0f;
    AfternoonPalette.SunYawDegrees = 210.0f;
    AfternoonPalette.SkyLightColor = FLinearColor(1.0f, 0.92f, 0.85f, 1.0f);
    AfternoonPalette.SkyLightIntensity = 2.0f;
    AfternoonPalette.FogColor = FLinearColor(0.95f, 0.8f, 0.6f, 1.0f);
    AfternoonPalette.FogDensity = 0.03f;
    AfternoonPalette.bVolumetricFog = true;

    // DUSK — Deep orange, purple volumetric fog
    DuskPalette.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskPalette.SunIntensity = 2.5f;
    DuskPalette.SunPitchDegrees = -8.0f;
    DuskPalette.SunYawDegrees = 270.0f;  // West
    DuskPalette.SkyLightColor = FLinearColor(0.6f, 0.5f, 0.8f, 1.0f);
    DuskPalette.SkyLightIntensity = 1.8f;
    DuskPalette.FogColor = FLinearColor(0.7f, 0.45f, 0.65f, 1.0f);
    DuskPalette.FogDensity = 0.04f;
    DuskPalette.bVolumetricFog = true;

    // NIGHT — Deep blue, stars, minimal fog
    NightPalette.SunColor = FLinearColor(0.2f, 0.25f, 0.45f, 1.0f);
    NightPalette.SunIntensity = 0.1f;
    NightPalette.SunPitchDegrees = -89.0f;
    NightPalette.SunYawDegrees = 0.0f;
    NightPalette.SkyLightColor = FLinearColor(0.15f, 0.2f, 0.4f, 1.0f);
    NightPalette.SkyLightIntensity = 0.3f;
    NightPalette.FogColor = FLinearColor(0.1f, 0.12f, 0.25f, 1.0f);
    NightPalette.FogDensity = 0.06f;
    NightPalette.bVolumetricFog = true;
}
