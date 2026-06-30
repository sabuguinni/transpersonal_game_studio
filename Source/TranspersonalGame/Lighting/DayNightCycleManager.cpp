#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

// ============================================================
//  Constructor — set default palettes
// ============================================================
ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz is enough for lighting

    // --- Dawn palette (warm pink-orange, low sun) ---
    DawnPalette.SunPitchDeg       = -8.0f;
    DawnPalette.SunYawDeg         = 90.0f;
    DawnPalette.SunIntensity      = 1.5f;
    DawnPalette.SunColor          = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    DawnPalette.FogDensity        = 0.06f;
    DawnPalette.FogColor          = FLinearColor(0.6f, 0.35f, 0.2f, 1.0f);
    DawnPalette.RayleighScale     = 0.055f;
    DawnPalette.MieScale          = 0.012f;
    DawnPalette.ColorTemperatureK = 3200.0f;

    // --- Morning palette (golden hour) ---
    MorningPalette.SunPitchDeg       = -25.0f;
    MorningPalette.SunYawDeg         = 120.0f;
    MorningPalette.SunIntensity      = 5.0f;
    MorningPalette.SunColor          = FLinearColor(1.0f, 0.85f, 0.55f, 1.0f);
    MorningPalette.FogDensity        = 0.025f;
    MorningPalette.FogColor          = FLinearColor(0.55f, 0.6f, 0.75f, 1.0f);
    MorningPalette.RayleighScale     = 0.04f;
    MorningPalette.MieScale          = 0.006f;
    MorningPalette.ColorTemperatureK = 4500.0f;

    // --- Midday palette (harsh white sun) ---
    MiddayPalette.SunPitchDeg       = -75.0f;
    MiddayPalette.SunYawDeg         = 180.0f;
    MiddayPalette.SunIntensity      = 12.0f;
    MiddayPalette.SunColor          = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddayPalette.FogDensity        = 0.015f;
    MiddayPalette.FogColor          = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    MiddayPalette.RayleighScale     = 0.0331f;
    MiddayPalette.MieScale          = 0.003f;
    MiddayPalette.ColorTemperatureK = 6500.0f;

    // --- Afternoon palette (warm directional) ---
    AfternoonPalette.SunPitchDeg       = -35.0f;
    AfternoonPalette.SunYawDeg         = 240.0f;
    AfternoonPalette.SunIntensity      = 7.0f;
    AfternoonPalette.SunColor          = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    AfternoonPalette.FogDensity        = 0.02f;
    AfternoonPalette.FogColor          = FLinearColor(0.5f, 0.5f, 0.65f, 1.0f);
    AfternoonPalette.RayleighScale     = 0.038f;
    AfternoonPalette.MieScale          = 0.005f;
    AfternoonPalette.ColorTemperatureK = 5000.0f;

    // --- Dusk palette (deep orange-red) ---
    DuskPalette.SunPitchDeg       = -5.0f;
    DuskPalette.SunYawDeg         = 270.0f;
    DuskPalette.SunIntensity      = 1.0f;
    DuskPalette.SunColor          = FLinearColor(1.0f, 0.35f, 0.1f, 1.0f);
    DuskPalette.FogDensity        = 0.05f;
    DuskPalette.FogColor          = FLinearColor(0.55f, 0.25f, 0.1f, 1.0f);
    DuskPalette.RayleighScale     = 0.06f;
    DuskPalette.MieScale          = 0.015f;
    DuskPalette.ColorTemperatureK = 2800.0f;

    // --- Night palette (cool blue moonlight) ---
    NightPalette.SunPitchDeg       = -15.0f;
    NightPalette.SunYawDeg         = 220.0f;
    NightPalette.SunIntensity      = 0.3f;
    NightPalette.SunColor          = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);
    NightPalette.FogDensity        = 0.04f;
    NightPalette.FogColor          = FLinearColor(0.01f, 0.02f, 0.08f, 1.0f);
    NightPalette.RayleighScale     = 0.033f;
    NightPalette.MieScale          = 0.003f;
    NightPalette.ColorTemperatureK = 6500.0f;

    // --- Deep Night palette (almost no light) ---
    DeepNightPalette.SunPitchDeg       = -5.0f;
    DeepNightPalette.SunYawDeg         = 0.0f;
    DeepNightPalette.SunIntensity      = 0.05f;
    DeepNightPalette.SunColor          = FLinearColor(0.4f, 0.45f, 0.7f, 1.0f);
    DeepNightPalette.FogDensity        = 0.06f;
    DeepNightPalette.FogColor          = FLinearColor(0.005f, 0.01f, 0.04f, 1.0f);
    DeepNightPalette.RayleighScale     = 0.033f;
    DeepNightPalette.MieScale          = 0.002f;
    DeepNightPalette.ColorTemperatureK = 7000.0f;
}

// ============================================================
//  BeginPlay
// ============================================================
void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    // Apply initial palette immediately
    ApplyPalette(GetPaletteForPhase(TimeToPhase(NormalizedTimeOfDay)));
}

// ============================================================
//  Tick
// ============================================================
void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bCycleActive)
    {
        AdvanceCycle(DeltaTime);
    }
}

// ============================================================
//  AutoFindLightActors — scan level for required actors
// ============================================================
void ADayNightCycleManager::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!SunMoonLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunMoonLight = *It;
            break;
        }
    }
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
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
}

// ============================================================
//  AdvanceCycle
// ============================================================
void ADayNightCycleManager::AdvanceCycle(float DeltaTime)
{
    if (DayDurationSeconds <= 0.0f) return;

    NormalizedTimeOfDay += DeltaTime / DayDurationSeconds;
    if (NormalizedTimeOfDay >= 1.0f)
    {
        NormalizedTimeOfDay -= 1.0f;
    }

    ELight_TimeOfDay NewPhase = TimeToPhase(NormalizedTimeOfDay);

    // Blend between current and next palette based on position within phase
    // Simple approach: lerp between two adjacent palettes
    float PhaseBlend = 0.0f;
    ELight_TimeOfDay NextPhase = static_cast<ELight_TimeOfDay>((static_cast<uint8>(NewPhase) + 1) % 7);

    // Compute blend within phase (0..1)
    // Phase boundaries (normalized 0-1): Dawn=0.2, Morning=0.3, Midday=0.45, Afternoon=0.6, Dusk=0.75, Night=0.85, DeepNight=1.0/0.0
    static const float PhaseBoundaries[7] = { 0.20f, 0.30f, 0.45f, 0.60f, 0.75f, 0.85f, 1.00f };
    static const float PhaseStarts[7]     = { 0.15f, 0.20f, 0.30f, 0.45f, 0.60f, 0.75f, 0.85f };

    uint8 PhaseIdx = static_cast<uint8>(NewPhase);
    float PhaseStart = PhaseStarts[PhaseIdx];
    float PhaseEnd   = PhaseBoundaries[PhaseIdx];
    float PhaseLen   = PhaseEnd - PhaseStart;
    if (PhaseLen > 0.0f)
    {
        PhaseBlend = FMath::Clamp((NormalizedTimeOfDay - PhaseStart) / PhaseLen, 0.0f, 1.0f);
    }

    FLight_SkyPalette CurPalette  = GetPaletteForPhase(NewPhase);
    FLight_SkyPalette NextPalette = GetPaletteForPhase(NextPhase);
    FLight_SkyPalette BlendedPalette = LerpPalettes(CurPalette, NextPalette, PhaseBlend);
    ApplyPalette(BlendedPalette);

    // Fire phase change event
    if (NewPhase != LastPhase)
    {
        CurrentPhase = NewPhase;
        LastPhase = NewPhase;
        OnPhaseChanged(NewPhase);
    }
}

// ============================================================
//  TimeToPhase
// ============================================================
ELight_TimeOfDay ADayNightCycleManager::TimeToPhase(float T) const
{
    if (T < 0.15f) return ELight_TimeOfDay::DeepNight;
    if (T < 0.20f) return ELight_TimeOfDay::Dawn;
    if (T < 0.30f) return ELight_TimeOfDay::Morning;
    if (T < 0.45f) return ELight_TimeOfDay::Midday;
    if (T < 0.60f) return ELight_TimeOfDay::Afternoon;
    if (T < 0.75f) return ELight_TimeOfDay::Dusk;
    if (T < 0.85f) return ELight_TimeOfDay::Night;
    return ELight_TimeOfDay::DeepNight;
}

// ============================================================
//  GetPaletteForPhase
// ============================================================
FLight_SkyPalette ADayNightCycleManager::GetPaletteForPhase(ELight_TimeOfDay Phase) const
{
    switch (Phase)
    {
        case ELight_TimeOfDay::Dawn:        return DawnPalette;
        case ELight_TimeOfDay::Morning:     return MorningPalette;
        case ELight_TimeOfDay::Midday:      return MiddayPalette;
        case ELight_TimeOfDay::Afternoon:   return AfternoonPalette;
        case ELight_TimeOfDay::Dusk:        return DuskPalette;
        case ELight_TimeOfDay::Night:       return NightPalette;
        case ELight_TimeOfDay::DeepNight:   return DeepNightPalette;
        default:                            return MiddayPalette;
    }
}

// ============================================================
//  LerpPalettes
// ============================================================
FLight_SkyPalette ADayNightCycleManager::LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const
{
    FLight_SkyPalette Out;
    Out.SunPitchDeg       = FMath::Lerp(A.SunPitchDeg,       B.SunPitchDeg,       Alpha);
    Out.SunYawDeg         = FMath::Lerp(A.SunYawDeg,         B.SunYawDeg,         Alpha);
    Out.SunIntensity      = FMath::Lerp(A.SunIntensity,       B.SunIntensity,      Alpha);
    Out.SunColor          = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Out.FogDensity        = FMath::Lerp(A.FogDensity,         B.FogDensity,        Alpha);
    Out.FogColor          = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Out.RayleighScale     = FMath::Lerp(A.RayleighScale,      B.RayleighScale,     Alpha);
    Out.MieScale          = FMath::Lerp(A.MieScale,           B.MieScale,          Alpha);
    Out.ColorTemperatureK = FMath::Lerp(A.ColorTemperatureK,  B.ColorTemperatureK, Alpha);
    return Out;
}

// ============================================================
//  ApplyPalette — push values to actual light actors
// ============================================================
void ADayNightCycleManager::ApplyPalette(const FLight_SkyPalette& Palette)
{
    // Sun / Moon directional light
    if (SunMoonLight)
    {
        SunMoonLight->SetActorRotation(FRotator(Palette.SunPitchDeg, Palette.SunYawDeg, 0.0f));
        UDirectionalLightComponent* DLC = SunMoonLight->GetComponent();
        if (DLC)
        {
            DLC->SetIntensity(Palette.SunIntensity);
            DLC->SetLightColor(Palette.SunColor);
            DLC->SetUseTemperature(true);
            DLC->SetTemperature(Palette.ColorTemperatureK);
        }
    }

    // Exponential height fog
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

// ============================================================
//  Public API
// ============================================================
void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    NormalizedTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    CurrentPhase = TimeToPhase(NormalizedTimeOfDay);
    ApplyPalette(GetPaletteForPhase(CurrentPhase));
}

void ADayNightCycleManager::SetPhaseImmediate(ELight_TimeOfDay Phase)
{
    CurrentPhase = Phase;
    LastPhase = Phase;
    ApplyPalette(GetPaletteForPhase(Phase));
}

float ADayNightCycleManager::GetInGameHour() const
{
    return NormalizedTimeOfDay * 24.0f;
}
