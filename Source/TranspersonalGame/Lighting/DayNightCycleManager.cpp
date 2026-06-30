#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 ticks/sec is enough for lighting

    InitializeDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneLights();

    // Apply initial palette immediately
    FLight_SkyPalette InitialPalette = GetCurrentPalette();
    ApplyPaletteToScene(InitialPalette);
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCycleActive || DayDurationSeconds <= 0.0f)
        return;

    ElapsedSeconds += DeltaTime;
    TimeOfDayNormalized = FMath::Fmod(ElapsedSeconds / DayDurationSeconds, 1.0f);

    FLight_SkyPalette CurrentPalette = GetCurrentPalette();
    ApplyPaletteToScene(CurrentPalette);
}

ELight_TimeOfDay ADayNightCycleManager::GetCurrentTimeOfDay() const
{
    // Map normalized time (0-1) to 24h (0-24)
    float Hours = TimeOfDayNormalized * 24.0f;

    if (Hours < 4.5f)  return ELight_TimeOfDay::Midnight;
    if (Hours < 6.0f)  return ELight_TimeOfDay::PreDawn;
    if (Hours < 7.5f)  return ELight_TimeOfDay::Dawn;
    if (Hours < 10.0f) return ELight_TimeOfDay::Morning;
    if (Hours < 14.0f) return ELight_TimeOfDay::Midday;
    if (Hours < 17.0f) return ELight_TimeOfDay::Afternoon;
    if (Hours < 18.5f) return ELight_TimeOfDay::GoldenHour;
    if (Hours < 20.0f) return ELight_TimeOfDay::Dusk;
    if (Hours < 21.5f) return ELight_TimeOfDay::Twilight;
    return ELight_TimeOfDay::Night;
}

FLight_SkyPalette ADayNightCycleManager::GetCurrentPalette() const
{
    float Hours = TimeOfDayNormalized * 24.0f;

    // Define key time points (normalized 0-1)
    // Midnight=0, PreDawn=0.22, Dawn=0.27, Midday=0.5, GoldenHour=0.74, Dusk=0.79, Night=0.9
    struct FKeyframe
    {
        float Time;
        const FLight_SkyPalette* Palette;
    };

    TArray<FKeyframe> Keyframes = {
        { 0.0f,  &NightPalette },
        { 0.22f, &PreDawnPalette },
        { 0.27f, &DawnPalette },
        { 0.5f,  &MiddayPalette },
        { 0.74f, &GoldenHourPalette },
        { 0.79f, &DuskPalette },
        { 0.9f,  &NightPalette },
        { 1.0f,  &NightPalette }
    };

    // Find surrounding keyframes
    for (int32 i = 0; i < Keyframes.Num() - 1; ++i)
    {
        if (TimeOfDayNormalized >= Keyframes[i].Time && TimeOfDayNormalized < Keyframes[i + 1].Time)
        {
            float Alpha = (TimeOfDayNormalized - Keyframes[i].Time) /
                          (Keyframes[i + 1].Time - Keyframes[i].Time);
            return LerpPalettes(*Keyframes[i].Palette, *Keyframes[i + 1].Palette, Alpha);
        }
    }

    return NightPalette;
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    TimeOfDayNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ElapsedSeconds = TimeOfDayNormalized * DayDurationSeconds;

    FLight_SkyPalette Palette = GetCurrentPalette();
    ApplyPaletteToScene(Palette);
}

void ADayNightCycleManager::ApplyPaletteToScene(const FLight_SkyPalette& Palette)
{
    // Apply sun rotation and color
    if (SunLight)
    {
        UDirectionalLightComponent* SunComp = SunLight->GetComponent();
        if (SunComp)
        {
            // Clamp pitch to never exceed -30° (CAP enforcement)
            float ClampedPitch = FMath::Min(Palette.SunPitchAngle, -30.0f);
            FRotator CurrentRot = SunLight->GetActorRotation();
            SunLight->SetActorRotation(FRotator(ClampedPitch, CurrentRot.Yaw, CurrentRot.Roll));

            SunComp->SetIntensity(Palette.SunIntensity);
            SunComp->SetLightColor(Palette.SunColor);
        }
    }

    // Apply fog settings
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply sky light
    if (SkyLightActor)
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        if (SkyComp)
        {
            SkyComp->SetIntensity(Palette.SkyLightIntensity);
            SkyComp->SetLightColor(Palette.SkyLightColor);
        }
    }
}

float ADayNightCycleManager::GetSunPitchForTime(float NormalizedTime) const
{
    // Sun arc: rises from -90 (below horizon) at dawn, peaks at -75 at noon, sets at -90 at dusk
    // We use a sine curve mapped to the day portion (0.25 to 0.75 normalized)
    float DayFraction = (NormalizedTime - 0.25f) / 0.5f; // 0 at dawn, 1 at dusk
    DayFraction = FMath::Clamp(DayFraction, 0.0f, 1.0f);

    // Sine gives 0 at edges, 1 at peak
    float SinValue = FMath::Sin(DayFraction * PI);

    // Map: 0 = -90 (horizon), 1 = -75 (near zenith but clamped)
    float Pitch = FMath::Lerp(-90.0f, -75.0f, SinValue);

    // CAP enforcement: never above -30
    return FMath::Min(Pitch, -30.0f);
}

float ADayNightCycleManager::GetHours() const
{
    return TimeOfDayNormalized * 24.0f;
}

FString ADayNightCycleManager::GetTimeString() const
{
    float Hours = GetHours();
    int32 H = FMath::FloorToInt(Hours);
    int32 M = FMath::FloorToInt((Hours - H) * 60.0f);
    return FString::Printf(TEXT("%02d:%02d"), H, M);
}

void ADayNightCycleManager::InitializeDefaultPalettes()
{
    // === PRE-DAWN: deep blue-violet, sun below horizon ===
    PreDawnPalette.SunColor = FLinearColor(0.3f, 0.25f, 0.5f, 1.0f);
    PreDawnPalette.SunIntensity = 0.1f;
    PreDawnPalette.SunPitchAngle = -88.0f;
    PreDawnPalette.FogColor = FLinearColor(0.08f, 0.06f, 0.18f, 1.0f);
    PreDawnPalette.FogDensity = 0.045f;
    PreDawnPalette.SkyLightColor = FLinearColor(0.15f, 0.12f, 0.3f, 1.0f);
    PreDawnPalette.SkyLightIntensity = 0.3f;
    PreDawnPalette.VolumetricFogExtinction = 2.0f;

    // === DAWN: warm pink-orange horizon ===
    DawnPalette.SunColor = FLinearColor(1.0f, 0.55f, 0.2f, 1.0f);
    DawnPalette.SunIntensity = 2.5f;
    DawnPalette.SunPitchAngle = -82.0f;
    DawnPalette.FogColor = FLinearColor(0.6f, 0.35f, 0.2f, 1.0f);
    DawnPalette.FogDensity = 0.04f;
    DawnPalette.SkyLightColor = FLinearColor(0.7f, 0.5f, 0.4f, 1.0f);
    DawnPalette.SkyLightIntensity = 0.7f;
    DawnPalette.VolumetricFogExtinction = 1.5f;

    // === MIDDAY: harsh white-yellow overhead sun ===
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.97f, 0.85f, 1.0f);
    MiddayPalette.SunIntensity = 12.0f;
    MiddayPalette.SunPitchAngle = -75.0f;
    MiddayPalette.FogColor = FLinearColor(0.55f, 0.65f, 0.8f, 1.0f);
    MiddayPalette.FogDensity = 0.015f;
    MiddayPalette.SkyLightColor = FLinearColor(0.8f, 0.88f, 1.0f, 1.0f);
    MiddayPalette.SkyLightIntensity = 1.4f;
    MiddayPalette.VolumetricFogExtinction = 0.8f;

    // === GOLDEN HOUR: warm amber-gold long shadows ===
    GoldenHourPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.22f, 1.0f);
    GoldenHourPalette.SunIntensity = 5.0f;
    GoldenHourPalette.SunPitchAngle = -35.0f;
    GoldenHourPalette.FogColor = FLinearColor(0.75f, 0.45f, 0.15f, 1.0f);
    GoldenHourPalette.FogDensity = 0.028f;
    GoldenHourPalette.SkyLightColor = FLinearColor(0.9f, 0.7f, 0.45f, 1.0f);
    GoldenHourPalette.SkyLightIntensity = 1.0f;
    GoldenHourPalette.VolumetricFogExtinction = 1.3f;

    // === DUSK: orange-purple, sun at horizon ===
    DuskPalette.SunColor = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskPalette.SunIntensity = 1.2f;
    DuskPalette.SunPitchAngle = -30.0f;
    DuskPalette.FogColor = FLinearColor(0.55f, 0.22f, 0.08f, 1.0f);
    DuskPalette.FogDensity = 0.035f;
    DuskPalette.SkyLightColor = FLinearColor(0.55f, 0.38f, 0.7f, 1.0f);
    DuskPalette.SkyLightIntensity = 0.9f;
    DuskPalette.VolumetricFogExtinction = 1.6f;

    // === NIGHT: deep blue-black, moonlight ===
    NightPalette.SunColor = FLinearColor(0.05f, 0.06f, 0.15f, 1.0f);
    NightPalette.SunIntensity = 0.05f;
    NightPalette.SunPitchAngle = -90.0f;
    NightPalette.FogColor = FLinearColor(0.02f, 0.03f, 0.08f, 1.0f);
    NightPalette.FogDensity = 0.055f;
    NightPalette.SkyLightColor = FLinearColor(0.08f, 0.1f, 0.22f, 1.0f);
    NightPalette.SkyLightIntensity = 0.15f;
    NightPalette.VolumetricFogExtinction = 2.5f;
}

void ADayNightCycleManager::AutoFindSceneLights()
{
    if (!GetWorld()) return;

    // Auto-find DirectionalLight if not assigned
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Auto-find SkyLight if not assigned
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    // Auto-find ExponentialHeightFog if not assigned
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: Sun=%s, Sky=%s, Fog=%s"),
        SunLight ? *SunLight->GetName() : TEXT("NOT FOUND"),
        SkyLightActor ? *SkyLightActor->GetName() : TEXT("NOT FOUND"),
        HeightFog ? *HeightFog->GetName() : TEXT("NOT FOUND"));
}

FLight_SkyPalette ADayNightCycleManager::LerpPalettes(const FLight_SkyPalette& A, const FLight_SkyPalette& B, float Alpha) const
{
    FLight_SkyPalette Result;
    Result.SunColor = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f
    );
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunPitchAngle = FMath::Lerp(A.SunPitchAngle, B.SunPitchAngle, Alpha);
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
    Result.VolumetricFogExtinction = FMath::Lerp(A.VolumetricFogExtinction, B.VolumetricFogExtinction, Alpha);
    return Result;
}
