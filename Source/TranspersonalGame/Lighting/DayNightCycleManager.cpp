#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20fps tick for lighting — sufficient for smooth transitions

    // Default Dawn Palette
    DawnPalette.SunColor        = FLinearColor(1.f, 0.65f, 0.2f, 1.f);
    DawnPalette.SunIntensity    = 3.5f;
    DawnPalette.SunPitchDegrees = 8.f;
    DawnPalette.FogInscatterColor = FLinearColor(0.85f, 0.6f, 0.35f, 1.f);
    DawnPalette.FogDensity      = 0.035f;
    DawnPalette.SkyLightColor   = FLinearColor(0.5f, 0.45f, 0.6f, 1.f);
    DawnPalette.SkyLightIntensity = 0.6f;

    // Morning Palette
    MorningPalette.SunColor        = FLinearColor(1.f, 0.85f, 0.6f, 1.f);
    MorningPalette.SunIntensity    = 7.f;
    MorningPalette.SunPitchDegrees = 30.f;
    MorningPalette.FogInscatterColor = FLinearColor(0.7f, 0.75f, 0.9f, 1.f);
    MorningPalette.FogDensity      = 0.02f;
    MorningPalette.SkyLightColor   = FLinearColor(0.55f, 0.6f, 0.75f, 1.f);
    MorningPalette.SkyLightIntensity = 0.9f;

    // Midday Palette
    MiddayPalette.SunColor        = FLinearColor(1.f, 0.98f, 0.9f, 1.f);
    MiddayPalette.SunIntensity    = 12.f;
    MiddayPalette.SunPitchDegrees = 75.f;
    MiddayPalette.FogInscatterColor = FLinearColor(0.5f, 0.65f, 0.85f, 1.f);
    MiddayPalette.FogDensity      = 0.01f;
    MiddayPalette.SkyLightColor   = FLinearColor(0.6f, 0.7f, 0.9f, 1.f);
    MiddayPalette.SkyLightIntensity = 1.2f;

    // Afternoon Palette
    AfternoonPalette.SunColor        = FLinearColor(1.f, 0.88f, 0.55f, 1.f);
    AfternoonPalette.SunIntensity    = 9.f;
    AfternoonPalette.SunPitchDegrees = 40.f;
    AfternoonPalette.FogInscatterColor = FLinearColor(0.75f, 0.7f, 0.8f, 1.f);
    AfternoonPalette.FogDensity      = 0.018f;
    AfternoonPalette.SkyLightColor   = FLinearColor(0.6f, 0.55f, 0.65f, 1.f);
    AfternoonPalette.SkyLightIntensity = 1.0f;

    // Dusk Palette
    DuskPalette.SunColor        = FLinearColor(1.f, 0.45f, 0.1f, 1.f);
    DuskPalette.SunIntensity    = 4.f;
    DuskPalette.SunPitchDegrees = -5.f;
    DuskPalette.FogInscatterColor = FLinearColor(0.9f, 0.45f, 0.2f, 1.f);
    DuskPalette.FogDensity      = 0.04f;
    DuskPalette.SkyLightColor   = FLinearColor(0.4f, 0.3f, 0.5f, 1.f);
    DuskPalette.SkyLightIntensity = 0.5f;

    // Night Palette
    NightPalette.SunColor        = FLinearColor(0.15f, 0.2f, 0.45f, 1.f);
    NightPalette.SunIntensity    = 0.3f;
    NightPalette.SunPitchDegrees = -60.f;
    NightPalette.FogInscatterColor = FLinearColor(0.05f, 0.08f, 0.2f, 1.f);
    NightPalette.FogDensity      = 0.06f;
    NightPalette.SkyLightColor   = FLinearColor(0.1f, 0.12f, 0.25f, 1.f);
    NightPalette.SkyLightIntensity = 0.2f;
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyCurrentPaletteNow();
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bPauseDayNightCycle)
    {
        TickDayNightCycle(DeltaTime);
    }
}

void ADayNightCycleManager::TickDayNightCycle(float DeltaTime)
{
    // Advance game time
    const float SecondsPerGameDay = DayDurationMinutes * 60.f;
    const float HoursPerSecond = 24.f / SecondsPerGameDay;
    CurrentHour += DeltaTime * HoursPerSecond;

    if (CurrentHour >= 24.f)
    {
        CurrentHour -= 24.f;
    }

    // Determine current time of day
    ELight_TimeOfDay NewTimeOfDay = GetTimeOfDayFromHour(CurrentHour);
    if (NewTimeOfDay != PreviousTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        OnTimeOfDayChanged(NewTimeOfDay);
        PreviousTimeOfDay = NewTimeOfDay;
    }

    // Interpolate between palettes
    ELight_TimeOfDay NextTimeOfDay = static_cast<ELight_TimeOfDay>((static_cast<uint8>(CurrentTimeOfDay) + 1) % 8);
    float PhaseAlpha = FMath::Frac(CurrentHour / 3.f); // Rough alpha within phase
    FLight_SkyPalette CurrentPalette = GetPaletteForTime(CurrentTimeOfDay);
    FLight_SkyPalette NextPalette = GetPaletteForTime(NextTimeOfDay);
    FLight_SkyPalette BlendedPalette = LerpPalettes(CurrentPalette, NextPalette, PhaseAlpha);

    ApplyPaletteToWorld(BlendedPalette);
}

ELight_TimeOfDay ADayNightCycleManager::GetTimeOfDayFromHour(float Hour) const
{
    if (Hour >= 5.f  && Hour < 7.f)  return ELight_TimeOfDay::Dawn;
    if (Hour >= 7.f  && Hour < 10.f) return ELight_TimeOfDay::Morning;
    if (Hour >= 10.f && Hour < 14.f) return ELight_TimeOfDay::Midday;
    if (Hour >= 14.f && Hour < 17.f) return ELight_TimeOfDay::Afternoon;
    if (Hour >= 17.f && Hour < 19.f) return ELight_TimeOfDay::Dusk;
    if (Hour >= 19.f && Hour < 21.f) return ELight_TimeOfDay::Twilight;
    if (Hour >= 21.f && Hour < 24.f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::Midnight; // 0-5
}

FLight_SkyPalette ADayNightCycleManager::GetPaletteForTime(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:      return DawnPalette;
        case ELight_TimeOfDay::Morning:   return MorningPalette;
        case ELight_TimeOfDay::Midday:    return MiddayPalette;
        case ELight_TimeOfDay::Afternoon: return AfternoonPalette;
        case ELight_TimeOfDay::Dusk:      return DuskPalette;
        case ELight_TimeOfDay::Twilight:  return DuskPalette;  // Reuse dusk, slightly darker
        case ELight_TimeOfDay::Night:     return NightPalette;
        case ELight_TimeOfDay::Midnight:  return NightPalette;
        default:                          return MiddayPalette;
    }
}

FLight_SkyPalette ADayNightCycleManager::LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const
{
    FLight_SkyPalette Result;
    Result.SunColor           = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity       = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitchDegrees    = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.FogInscatterColor  = FLinearColor::LerpUsingHSV(A.FogInscatterColor, B.FogInscatterColor, Alpha);
    Result.FogDensity         = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.SkyLightColor      = FLinearColor::LerpUsingHSV(A.SkyLightColor, B.SkyLightColor, Alpha);
    Result.SkyLightIntensity  = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    return Result;
}

void ADayNightCycleManager::ApplyPaletteToWorld(const FLight_SkyPalette& Palette)
{
    // Apply to Directional Light (Sun)
    if (SunLight)
    {
        UDirectionalLightComponent* DirComp = SunLight->GetComponent();
        if (DirComp)
        {
            DirComp->SetIntensity(Palette.SunIntensity);
            DirComp->SetLightColor(Palette.SunColor);
        }
        FRotator SunRot = SunLight->GetActorRotation();
        SunLight->SetActorRotation(FRotator(Palette.SunPitchDegrees, SunRot.Yaw, SunRot.Roll));
        CurrentSunPitch = Palette.SunPitchDegrees;
    }

    // Apply to SkyLight
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
        UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogInscatterColor);
        }
    }
}

void ADayNightCycleManager::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    if (!FogActor)
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if ((*It)->FindComponentByClass<UExponentialHeightFogComponent>())
            {
                FogActor = *It;
                break;
            }
        }
    }
}

void ADayNightCycleManager::SetHour(float NewHour)
{
    CurrentHour = FMath::Clamp(NewHour, 0.f, 23.99f);
    CurrentTimeOfDay = GetTimeOfDayFromHour(CurrentHour);
    ApplyCurrentPaletteNow();
}

void ADayNightCycleManager::AdvanceTimeByHours(float Hours)
{
    SetHour(FMath::Fmod(CurrentHour + Hours, 24.f));
}

void ADayNightCycleManager::ApplyCurrentPaletteNow()
{
    AutoFindLightingActors();
    FLight_SkyPalette Palette = GetPaletteForTime(GetTimeOfDayFromHour(CurrentHour));
    ApplyPaletteToWorld(Palette);
}
