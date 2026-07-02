#include "DayNightCycleManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20fps tick — sufficient for lighting
    InitializeDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightActors();
    CurrentPhase = HourToPhase(CurrentGameHour);
    PreviousPhase = CurrentPhase;
    const FLight_DayPalette* Palette = FindPaletteForPhase(CurrentPhase);
    if (Palette)
    {
        CurrentBlendedPalette = *Palette;
        TargetPalette = *Palette;
        ApplyPalette(*Palette, 1.0f);
    }
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bCycleActive) return;
    UpdateLighting(DeltaTime);
}

void ADayNightCycleManager::UpdateLighting(float DeltaTime)
{
    // Advance game time
    const float HoursPerSecond = 1.0f / RealSecondsPerGameHour;
    CurrentGameHour += HoursPerSecond * DeltaTime;
    if (CurrentGameHour >= 24.0f) CurrentGameHour -= 24.0f;

    // Detect phase change
    ELight_DayPhase NewPhase = HourToPhase(CurrentGameHour);
    if (NewPhase != CurrentPhase)
    {
        PreviousPhase = CurrentPhase;
        CurrentPhase = NewPhase;
        PhaseBlendAlpha = 0.0f;

        const FLight_DayPalette* NewPalette = FindPaletteForPhase(CurrentPhase);
        if (NewPalette) TargetPalette = *NewPalette;

        OnPhaseChanged(CurrentPhase, PreviousPhase);
    }

    // Blend toward target palette
    PhaseBlendAlpha = FMath::Clamp(PhaseBlendAlpha + DeltaTime * TransitionBlendSpeed, 0.0f, 1.0f);
    ApplyPalette(TargetPalette, PhaseBlendAlpha);
}

void ADayNightCycleManager::ApplyPalette(const FLight_DayPalette& Palette, float BlendAlpha)
{
    // Blend sun pitch/yaw from current blended values
    float BlendedPitch = FMath::Lerp(CurrentBlendedPalette.SunPitch, Palette.SunPitch, BlendAlpha);
    float BlendedYaw   = FMath::Lerp(CurrentBlendedPalette.SunYaw,   Palette.SunYaw,   BlendAlpha);
    float BlendedIntensity = FMath::Lerp(CurrentBlendedPalette.SunIntensity, Palette.SunIntensity, BlendAlpha);
    FLinearColor BlendedSunColor = FLinearColor::LerpUsingHSV(CurrentBlendedPalette.SunColor, Palette.SunColor, BlendAlpha);
    float BlendedFogDensity = FMath::Lerp(CurrentBlendedPalette.FogDensity, Palette.FogDensity, BlendAlpha);
    FLinearColor BlendedFogColor = FLinearColor::LerpUsingHSV(CurrentBlendedPalette.FogColor, Palette.FogColor, BlendAlpha);
    float BlendedSkyIntensity = FMath::Lerp(CurrentBlendedPalette.SkyLightIntensity, Palette.SkyLightIntensity, BlendAlpha);

    // CAP: Sun pitch must never exceed -30° (guard against accidental day-sky during night)
    BlendedPitch = FMath::Min(BlendedPitch, -30.0f);
    CurrentSunPitch = BlendedPitch;
    CurrentSunYaw   = BlendedYaw;

    // Apply to DirectionalLight
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(BlendedPitch, BlendedYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(BlendedIntensity);
            DLC->SetLightColor(BlendedSunColor.ToFColor(true));
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(BlendedSkyIntensity);
            SLC->SetLightColor(FLinearColor::LerpUsingHSV(CurrentBlendedPalette.SkyLightColor, Palette.SkyLightColor, BlendAlpha).ToFColor(true));
        }
    }

    // Apply to ExponentialHeightFog
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(BlendedFogDensity);
            FogComp->SetFogInscatteringColor(BlendedFogColor);
        }
    }

    // Update blended state if fully transitioned
    if (BlendAlpha >= 1.0f)
    {
        CurrentBlendedPalette = Palette;
    }
}

ELight_DayPhase ADayNightCycleManager::HourToPhase(float Hour) const
{
    if (Hour >= 5.0f  && Hour < 7.0f)  return ELight_DayPhase::Dawn;
    if (Hour >= 7.0f  && Hour < 10.0f) return ELight_DayPhase::Morning;
    if (Hour >= 10.0f && Hour < 14.0f) return ELight_DayPhase::Midday;
    if (Hour >= 14.0f && Hour < 17.0f) return ELight_DayPhase::Afternoon;
    if (Hour >= 17.0f && Hour < 19.5f) return ELight_DayPhase::Dusk;
    if (Hour >= 19.5f && Hour < 22.0f) return ELight_DayPhase::Evening;
    if (Hour >= 22.0f || Hour < 2.0f)  return ELight_DayPhase::Night;
    return ELight_DayPhase::DeepNight;
}

const FLight_DayPalette* ADayNightCycleManager::FindPaletteForPhase(ELight_DayPhase Phase) const
{
    for (const FLight_DayPalette& P : DayPalettes)
    {
        if (P.Phase == Phase) return &P;
    }
    return DayPalettes.Num() > 0 ? &DayPalettes[0] : nullptr;
}

void ADayNightCycleManager::AutoFindLightActors()
{
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
    if (!FogActor)
    {
        for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It)
        {
            FogActor = *It;
            break;
        }
    }
}

void ADayNightCycleManager::SetGameHour(float NewHour)
{
    CurrentGameHour = FMath::Clamp(NewHour, 0.0f, 24.0f);
    CurrentPhase = HourToPhase(CurrentGameHour);
    const FLight_DayPalette* Palette = FindPaletteForPhase(CurrentPhase);
    if (Palette)
    {
        TargetPalette = *Palette;
        PhaseBlendAlpha = 0.0f;
    }
}

void ADayNightCycleManager::SetPhaseImmediate(ELight_DayPhase Phase)
{
    CurrentPhase = Phase;
    const FLight_DayPalette* Palette = FindPaletteForPhase(Phase);
    if (Palette)
    {
        CurrentBlendedPalette = *Palette;
        TargetPalette = *Palette;
        PhaseBlendAlpha = 1.0f;
        ApplyPalette(*Palette, 1.0f);
    }
}

FString ADayNightCycleManager::GetCurrentPhaseString() const
{
    switch (CurrentPhase)
    {
        case ELight_DayPhase::Dawn:      return TEXT("Dawn");
        case ELight_DayPhase::Morning:   return TEXT("Morning");
        case ELight_DayPhase::Midday:    return TEXT("Midday");
        case ELight_DayPhase::Afternoon: return TEXT("Afternoon");
        case ELight_DayPhase::Dusk:      return TEXT("Dusk");
        case ELight_DayPhase::Evening:   return TEXT("Evening");
        case ELight_DayPhase::Night:     return TEXT("Night");
        case ELight_DayPhase::DeepNight: return TEXT("Deep Night");
        default:                          return TEXT("Unknown");
    }
}

float ADayNightCycleManager::GetNormalizedTimeOfDay() const
{
    return CurrentGameHour / 24.0f;
}

bool ADayNightCycleManager::IsNightTime() const
{
    return CurrentPhase == ELight_DayPhase::Night ||
           CurrentPhase == ELight_DayPhase::DeepNight ||
           CurrentPhase == ELight_DayPhase::Evening;
}

bool ADayNightCycleManager::IsLowVisibility() const
{
    return IsNightTime() ||
           CurrentWeather == ELight_WeatherState::HeavyRain ||
           CurrentWeather == ELight_WeatherState::Fog ||
           CurrentWeather == ELight_WeatherState::Thunderstorm;
}

void ADayNightCycleManager::ApplyCurrentPaletteInEditor()
{
    AutoFindLightActors();
    const FLight_DayPalette* Palette = FindPaletteForPhase(CurrentPhase);
    if (Palette)
    {
        CurrentBlendedPalette = *Palette;
        ApplyPalette(*Palette, 1.0f);
    }
}

void ADayNightCycleManager::InitializeDefaultPalettes()
{
    DayPalettes.Empty();

    // DAWN — soft pink-orange, low sun, warm mist
    FLight_DayPalette Dawn;
    Dawn.Phase = ELight_DayPhase::Dawn;
    Dawn.SunPitch = -8.0f; Dawn.SunYaw = 90.0f;
    Dawn.SunIntensity = 2.5f;
    Dawn.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
    Dawn.SkyLightIntensity = 0.45f;
    Dawn.SkyLightColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f);
    Dawn.FogDensity = 0.038f;
    Dawn.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    Dawn.FogHeightFalloff = 0.18f;
    DayPalettes.Add(Dawn);

    // MORNING — warm golden light, low angle, long shadows
    FLight_DayPalette Morning;
    Morning.Phase = ELight_DayPhase::Morning;
    Morning.SunPitch = -35.0f; Morning.SunYaw = 120.0f;
    Morning.SunIntensity = 7.5f;
    Morning.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    Morning.SkyLightIntensity = 0.8f;
    Morning.SkyLightColor = FLinearColor(0.5f, 0.65f, 1.0f, 1.0f);
    Morning.FogDensity = 0.022f;
    Morning.FogColor = FLinearColor(0.7f, 0.75f, 0.9f, 1.0f);
    Morning.FogHeightFalloff = 0.20f;
    DayPalettes.Add(Morning);

    // MIDDAY — harsh overhead white sun, deep blue sky
    FLight_DayPalette Midday;
    Midday.Phase = ELight_DayPhase::Midday;
    Midday.SunPitch = -78.0f; Midday.SunYaw = 180.0f;
    Midday.SunIntensity = 12.0f;
    Midday.SunColor = FLinearColor(1.0f, 0.97f, 0.92f, 1.0f);
    Midday.SkyLightIntensity = 1.2f;
    Midday.SkyLightColor = FLinearColor(0.35f, 0.55f, 1.0f, 1.0f);
    Midday.FogDensity = 0.012f;
    Midday.FogColor = FLinearColor(0.55f, 0.65f, 0.85f, 1.0f);
    Midday.FogHeightFalloff = 0.25f;
    DayPalettes.Add(Midday);

    // AFTERNOON — warm amber, slight haze
    FLight_DayPalette Afternoon;
    Afternoon.Phase = ELight_DayPhase::Afternoon;
    Afternoon.SunPitch = -48.0f; Afternoon.SunYaw = 240.0f;
    Afternoon.SunIntensity = 9.0f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.85f, 0.55f, 1.0f);
    Afternoon.SkyLightIntensity = 0.95f;
    Afternoon.SkyLightColor = FLinearColor(0.45f, 0.6f, 0.95f, 1.0f);
    Afternoon.FogDensity = 0.018f;
    Afternoon.FogColor = FLinearColor(0.75f, 0.7f, 0.6f, 1.0f);
    Afternoon.FogHeightFalloff = 0.22f;
    DayPalettes.Add(Afternoon);

    // DUSK — deep orange-red, dramatic silhouettes
    FLight_DayPalette Dusk;
    Dusk.Phase = ELight_DayPhase::Dusk;
    Dusk.SunPitch = -12.0f; Dusk.SunYaw = 270.0f;
    Dusk.SunIntensity = 3.8f;
    Dusk.SunColor = FLinearColor(1.0f, 0.38f, 0.08f, 1.0f);
    Dusk.SkyLightIntensity = 0.55f;
    Dusk.SkyLightColor = FLinearColor(0.7f, 0.35f, 0.2f, 1.0f);
    Dusk.FogDensity = 0.028f;
    Dusk.FogColor = FLinearColor(0.85f, 0.45f, 0.2f, 1.0f);
    Dusk.FogHeightFalloff = 0.18f;
    DayPalettes.Add(Dusk);

    // EVENING — purple-blue twilight, stars beginning
    FLight_DayPalette Evening;
    Evening.Phase = ELight_DayPhase::Evening;
    Evening.SunPitch = -32.0f; Evening.SunYaw = 285.0f;
    Evening.SunIntensity = 1.2f;
    Evening.SunColor = FLinearColor(0.4f, 0.3f, 0.7f, 1.0f);
    Evening.SkyLightIntensity = 0.3f;
    Evening.SkyLightColor = FLinearColor(0.2f, 0.2f, 0.55f, 1.0f);
    Evening.FogDensity = 0.032f;
    Evening.FogColor = FLinearColor(0.15f, 0.15f, 0.4f, 1.0f);
    Evening.FogHeightFalloff = 0.20f;
    DayPalettes.Add(Evening);

    // NIGHT — deep blue moonlight, bioluminescent hints
    FLight_DayPalette Night;
    Night.Phase = ELight_DayPhase::Night;
    Night.SunPitch = -72.0f; Night.SunYaw = 185.0f;
    Night.SunIntensity = 0.4f;
    Night.SunColor = FLinearColor(0.08f, 0.12f, 0.32f, 1.0f);
    Night.SkyLightIntensity = 0.35f;
    Night.SkyLightColor = FLinearColor(0.06f, 0.08f, 0.24f, 1.0f);
    Night.FogDensity = 0.042f;
    Night.FogColor = FLinearColor(0.02f, 0.04f, 0.12f, 1.0f);
    Night.FogHeightFalloff = 0.22f;
    DayPalettes.Add(Night);

    // DEEP NIGHT — near-total darkness, maximum danger
    FLight_DayPalette DeepNight;
    DeepNight.Phase = ELight_DayPhase::DeepNight;
    DeepNight.SunPitch = -85.0f; DeepNight.SunYaw = 0.0f;
    DeepNight.SunIntensity = 0.15f;
    DeepNight.SunColor = FLinearColor(0.04f, 0.06f, 0.18f, 1.0f);
    DeepNight.SkyLightIntensity = 0.18f;
    DeepNight.SkyLightColor = FLinearColor(0.03f, 0.04f, 0.15f, 1.0f);
    DeepNight.FogDensity = 0.055f;
    DeepNight.FogColor = FLinearColor(0.01f, 0.02f, 0.08f, 1.0f);
    DeepNight.FogHeightFalloff = 0.25f;
    DayPalettes.Add(DeepNight);
}
