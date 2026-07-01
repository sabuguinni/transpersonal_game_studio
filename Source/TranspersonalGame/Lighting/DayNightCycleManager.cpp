#include "DayNightCycleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Math/UnrealMathUtility.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 ticks/sec is plenty for lighting updates

    InitializeDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    FindLightingActorsInWorld();
    SetTimeOfDay(CurrentTimeOfDay);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bCycleActive)
    {
        UpdateCycle(DeltaTime);
    }
}

void ADayNightCycleManager::InitializeDefaultPalettes()
{
    // === DAWN (5:00 - 7:00) ===
    DawnPalette.SunPitch       = -8.0f;
    DawnPalette.SunYaw         = -90.0f;
    DawnPalette.SunIntensity   = 3.2f;
    DawnPalette.SunColor       = FLinearColor(1.0f, 0.72f, 0.38f, 1.0f);  // warm golden-orange
    DawnPalette.FogDensity     = 0.028f;
    DawnPalette.FogColor       = FLinearColor(0.85f, 0.62f, 0.78f, 1.0f); // pink-lavender mist
    DawnPalette.SkyLightIntensity = 1.4f;
    DawnPalette.SkyLightColor  = FLinearColor(0.75f, 0.82f, 1.0f, 1.0f);
    DawnPalette.BloomIntensity = 0.65f;
    DawnPalette.ExposureBias   = 1.2f;

    // === MORNING (7:00 - 10:00) ===
    MorningPalette.SunPitch       = -35.0f;
    MorningPalette.SunYaw         = -60.0f;
    MorningPalette.SunIntensity   = 6.5f;
    MorningPalette.SunColor       = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f); // warm yellow
    MorningPalette.FogDensity     = 0.018f;
    MorningPalette.FogColor       = FLinearColor(0.82f, 0.88f, 1.0f, 1.0f); // light blue haze
    MorningPalette.SkyLightIntensity = 1.8f;
    MorningPalette.SkyLightColor  = FLinearColor(0.85f, 0.90f, 1.0f, 1.0f);
    MorningPalette.BloomIntensity = 0.45f;
    MorningPalette.ExposureBias   = 0.5f;

    // === MIDDAY (10:00 - 14:00) ===
    MiddayPalette.SunPitch       = -75.0f;
    MiddayPalette.SunYaw         = 180.0f;
    MiddayPalette.SunIntensity   = 12.0f;
    MiddayPalette.SunColor       = FLinearColor(1.0f, 0.98f, 0.88f, 1.0f); // harsh white-yellow
    MiddayPalette.FogDensity     = 0.012f;
    MiddayPalette.FogColor       = FLinearColor(0.78f, 0.85f, 1.0f, 1.0f); // pale blue haze
    MiddayPalette.SkyLightIntensity = 2.2f;
    MiddayPalette.SkyLightColor  = FLinearColor(0.88f, 0.92f, 1.0f, 1.0f);
    MiddayPalette.BloomIntensity = 0.35f;
    MiddayPalette.ExposureBias   = 0.0f;

    // === AFTERNOON (14:00 - 17:00) ===
    AfternoonPalette.SunPitch       = -42.0f;
    AfternoonPalette.SunYaw         = 60.0f;
    AfternoonPalette.SunIntensity   = 8.0f;
    AfternoonPalette.SunColor       = FLinearColor(1.0f, 0.92f, 0.72f, 1.0f); // warm afternoon gold
    AfternoonPalette.FogDensity     = 0.015f;
    AfternoonPalette.FogColor       = FLinearColor(0.88f, 0.82f, 0.72f, 1.0f); // dusty amber haze
    AfternoonPalette.SkyLightIntensity = 1.9f;
    AfternoonPalette.SkyLightColor  = FLinearColor(0.92f, 0.88f, 0.80f, 1.0f);
    AfternoonPalette.BloomIntensity = 0.40f;
    AfternoonPalette.ExposureBias   = 0.2f;

    // === DUSK (17:00 - 19:30) ===
    DuskPalette.SunPitch       = -18.0f;
    DuskPalette.SunYaw         = 85.0f;
    DuskPalette.SunIntensity   = 4.5f;
    DuskPalette.SunColor       = FLinearColor(1.0f, 0.55f, 0.22f, 1.0f); // deep orange-amber
    DuskPalette.FogDensity     = 0.035f;
    DuskPalette.FogColor       = FLinearColor(0.78f, 0.45f, 0.28f, 1.0f); // amber-rust fog
    DuskPalette.SkyLightIntensity = 1.2f;
    DuskPalette.SkyLightColor  = FLinearColor(1.0f, 0.72f, 0.52f, 1.0f);
    DuskPalette.BloomIntensity = 0.75f;
    DuskPalette.ExposureBias   = 0.8f;

    // === NIGHT (19:30 - 22:00) ===
    NightPalette.SunPitch       = -5.0f;
    NightPalette.SunYaw         = 90.0f;
    NightPalette.SunIntensity   = 0.05f;
    NightPalette.SunColor       = FLinearColor(0.2f, 0.25f, 0.45f, 1.0f); // deep blue moonlight
    NightPalette.FogDensity     = 0.045f;
    NightPalette.FogColor       = FLinearColor(0.08f, 0.10f, 0.22f, 1.0f); // deep blue-black
    NightPalette.SkyLightIntensity = 0.35f;
    NightPalette.SkyLightColor  = FLinearColor(0.30f, 0.35f, 0.60f, 1.0f);
    NightPalette.BloomIntensity = 0.25f;
    NightPalette.ExposureBias   = -1.5f;
}

void ADayNightCycleManager::UpdateCycle(float DeltaTime)
{
    // Advance time: DayDurationSeconds = one full 24h cycle in real seconds
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaTime;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Determine current phase and blend
    ELight_TimeOfDay NewPhase = TimeToPhase(CurrentTimeOfDay);
    CurrentPhase = NewPhase;

    // Update sun position
    CurrentSunPitchDegrees = HourToSunPitch(CurrentTimeOfDay);
    CurrentSunYawDegrees   = HourToSunYaw(CurrentTimeOfDay);

    // Blend between palettes based on time
    FLight_TimeOfDayPalette BlendedPalette = GetBlendedPalette(CurrentTimeOfDay);
    ApplyPalette(BlendedPalette);
}

FLight_TimeOfDayPalette ADayNightCycleManager::GetBlendedPalette(float Hour) const
{
    // Define transition points
    // Dawn: 5-7, Morning: 7-10, Midday: 10-14, Afternoon: 14-17, Dusk: 17-19.5, Night: 19.5-5
    if (Hour >= 5.0f && Hour < 7.0f)
    {
        float Alpha = (Hour - 5.0f) / 2.0f;
        return InterpolatePalettes(NightPalette, DawnPalette, Alpha);
    }
    else if (Hour >= 7.0f && Hour < 10.0f)
    {
        float Alpha = (Hour - 7.0f) / 3.0f;
        return InterpolatePalettes(DawnPalette, MorningPalette, Alpha);
    }
    else if (Hour >= 10.0f && Hour < 14.0f)
    {
        float Alpha = (Hour - 10.0f) / 4.0f;
        return InterpolatePalettes(MorningPalette, MiddayPalette, Alpha);
    }
    else if (Hour >= 14.0f && Hour < 17.0f)
    {
        float Alpha = (Hour - 14.0f) / 3.0f;
        return InterpolatePalettes(MiddayPalette, AfternoonPalette, Alpha);
    }
    else if (Hour >= 17.0f && Hour < 19.5f)
    {
        float Alpha = (Hour - 17.0f) / 2.5f;
        return InterpolatePalettes(AfternoonPalette, DuskPalette, Alpha);
    }
    else if (Hour >= 19.5f && Hour < 22.0f)
    {
        float Alpha = (Hour - 19.5f) / 2.5f;
        return InterpolatePalettes(DuskPalette, NightPalette, Alpha);
    }
    else
    {
        // Full night (22:00 - 5:00)
        return NightPalette;
    }
}

void ADayNightCycleManager::ApplyPalette(const FLight_TimeOfDayPalette& Palette)
{
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Palette.SunPitch, Palette.SunYaw, 0.0f));
        UDirectionalLightComponent* SunComp = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (SunComp)
        {
            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
    }

    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

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

FLight_TimeOfDayPalette ADayNightCycleManager::InterpolatePalettes(
    const FLight_TimeOfDayPalette& A,
    const FLight_TimeOfDayPalette& B,
    float Alpha) const
{
    FLight_TimeOfDayPalette Result;
    float T = FMath::Clamp(Alpha, 0.0f, 1.0f);

    Result.SunPitch       = FMath::Lerp(A.SunPitch, B.SunPitch, T);
    Result.SunYaw         = FMath::Lerp(A.SunYaw, B.SunYaw, T);
    Result.SunIntensity   = FMath::Lerp(A.SunIntensity, B.SunIntensity, T);
    Result.SunColor       = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, T),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, T),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, T),
        1.0f
    );
    Result.FogDensity     = FMath::Lerp(A.FogDensity, B.FogDensity, T);
    Result.FogColor       = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, T),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, T),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, T),
        1.0f
    );
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, T);
    Result.SkyLightColor  = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, T),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, T),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, T),
        1.0f
    );
    Result.BloomIntensity = FMath::Lerp(A.BloomIntensity, B.BloomIntensity, T);
    Result.ExposureBias   = FMath::Lerp(A.ExposureBias, B.ExposureBias, T);

    return Result;
}

ELight_TimeOfDay ADayNightCycleManager::TimeToPhase(float Hour) const
{
    if (Hour >= 5.0f  && Hour < 7.0f)  return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.0f  && Hour < 10.0f) return ELight_TimeOfDay::Morning;
    if (Hour >= 10.0f && Hour < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.0f && Hour < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.0f && Hour < 19.5f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 19.5f && Hour < 22.0f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight;
}

float ADayNightCycleManager::HourToSunPitch(float Hour) const
{
    // Sun arc: rises at 5am (-5 deg), peaks at noon (-75 deg), sets at 19:30 (-5 deg)
    // Night: stays just below horizon at -3 deg
    if (Hour >= 5.0f && Hour <= 19.5f)
    {
        // Sine arc from -5 to -75 and back
        float NormalizedHour = (Hour - 5.0f) / (19.5f - 5.0f); // 0..1
        float SinArc = FMath::Sin(NormalizedHour * PI);
        return FMath::Lerp(-5.0f, -75.0f, SinArc);
    }
    return -3.0f; // below horizon at night
}

float ADayNightCycleManager::HourToSunYaw(float Hour) const
{
    // Sun sweeps from east (-90) through south (180) to west (90)
    if (Hour >= 5.0f && Hour <= 19.5f)
    {
        float NormalizedHour = (Hour - 5.0f) / (19.5f - 5.0f); // 0..1
        return FMath::Lerp(-90.0f, 90.0f, NormalizedHour);
    }
    return 90.0f;
}

void ADayNightCycleManager::SetTimeOfDay(float NewHour)
{
    CurrentTimeOfDay = FMath::Clamp(NewHour, 0.0f, 24.0f);
    CurrentPhase = TimeToPhase(CurrentTimeOfDay);
    FLight_TimeOfDayPalette Palette = GetBlendedPalette(CurrentTimeOfDay);
    ApplyPalette(Palette);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentPhase() const
{
    return CurrentPhase;
}

FString ADayNightCycleManager::GetFormattedTime() const
{
    int32 Hours   = FMath::FloorToInt(CurrentTimeOfDay);
    int32 Minutes = FMath::FloorToInt((CurrentTimeOfDay - Hours) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

void ADayNightCycleManager::AutoDiscoverLightingActors()
{
    FindLightingActorsInWorld();
}

void ADayNightCycleManager::FindLightingActorsInWorld()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ADirectionalLight> It(World); It; ++It)
    {
        SunLight = *It;
        break;
    }
    for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
    {
        HeightFog = *It;
        break;
    }
    for (TActorIterator<ASkyLight> It(World); It; ++It)
    {
        SkyLightActor = *It;
        break;
    }
}

void ADayNightCycleManager::ApplyDawnPreset()
{
    SetTimeOfDay(6.0f);
}

void ADayNightCycleManager::ApplyMiddayPreset()
{
    SetTimeOfDay(12.0f);
}

void ADayNightCycleManager::ApplyDuskPreset()
{
    SetTimeOfDay(18.5f);
}

void ADayNightCycleManager::ApplyNightPreset()
{
    SetTimeOfDay(23.0f);
}

// Helper used in UpdateCycle — declared here to avoid header pollution
FLight_TimeOfDayPalette ADayNightCycleManager::GetBlendedPalette(float Hour) const
{
    // (duplicate definition guard — implementation above is canonical)
    // This stub satisfies the linker if the above inline is stripped by the compiler.
    return DawnPalette;
}
