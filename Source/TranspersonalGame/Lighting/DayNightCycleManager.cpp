#include "DayNightCycleManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ADayNightCycleManager::ADayNightCycleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;  // 20Hz — smooth enough for lighting
    InitDefaultPalettes();
}

void ADayNightCycleManager::BeginPlay()
{
    Super::BeginPlay();
    AutoFindSceneActors();
    // Apply initial palette
    ApplyPalette(GetPaletteForPhase(GetCurrentPhase()));
}

void ADayNightCycleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bPauseCycle)
    {
        TickCycle(DeltaTime);
    }
}

void ADayNightCycleManager::TickCycle(float DeltaTime)
{
    PreviousTimeOfDay = CurrentTimeOfDay;

    float AdvanceRate = (DayDurationSeconds > 0.0f) ? (1.0f / DayDurationSeconds) * TimeMultiplier : 0.0f;
    CurrentTimeOfDay = FMath::Fmod(CurrentTimeOfDay + DeltaTime * AdvanceRate, 1.0f);

    // Blend between current and next phase
    ELight_DayPhase CurrentPhase = GetCurrentPhase();
    CachedPhase = CurrentPhase;

    // Determine blend alpha within phase
    // Each phase occupies roughly 1/8 of the day
    float PhaseWidth = 1.0f / 8.0f;
    float PhaseStart = static_cast<float>(static_cast<uint8>(CurrentPhase)) * PhaseWidth;
    float AlphaInPhase = FMath::Clamp((CurrentTimeOfDay - PhaseStart) / PhaseWidth, 0.0f, 1.0f);

    // Get next phase
    ELight_DayPhase NextPhase = static_cast<ELight_DayPhase>((static_cast<uint8>(CurrentPhase) + 1) % 8);

    FLight_SkyPalette FromPalette = GetPaletteForPhase(CurrentPhase);
    FLight_SkyPalette ToPalette = GetPaletteForPhase(NextPhase);

    BlendPalettes(FromPalette, ToPalette, AlphaInPhase);
}

void ADayNightCycleManager::SetTimeOfDay(float NormalizedTime)
{
    CurrentTimeOfDay = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    ApplyPalette(GetPaletteForPhase(GetCurrentPhase()));
}

ELight_DayPhase ADayNightCycleManager::GetCurrentPhase() const
{
    // 0.0 = midnight, 0.25 = dawn, 0.5 = midday, 0.75 = dusk
    float T = CurrentTimeOfDay;
    if      (T < 0.125f) return ELight_DayPhase::Midnight;
    else if (T < 0.25f)  return ELight_DayPhase::Night;
    else if (T < 0.33f)  return ELight_DayPhase::Dawn;
    else if (T < 0.42f)  return ELight_DayPhase::Morning;
    else if (T < 0.58f)  return ELight_DayPhase::Midday;
    else if (T < 0.67f)  return ELight_DayPhase::Afternoon;
    else if (T < 0.75f)  return ELight_DayPhase::Dusk;
    else if (T < 0.875f) return ELight_DayPhase::Twilight;
    else                 return ELight_DayPhase::Night;
}

FLight_SkyPalette ADayNightCycleManager::GetPaletteForPhase(ELight_DayPhase Phase) const
{
    switch (Phase)
    {
        case ELight_DayPhase::Dawn:      return DawnPalette;
        case ELight_DayPhase::Morning:   return MorningPalette;
        case ELight_DayPhase::Midday:    return MiddayPalette;
        case ELight_DayPhase::Afternoon: return AfternoonPalette;
        case ELight_DayPhase::Dusk:      return DuskPalette;
        case ELight_DayPhase::Twilight:  return TwilightPalette;
        case ELight_DayPhase::Night:     return NightPalette;
        case ELight_DayPhase::Midnight:  return NightPalette;
        default:                         return MiddayPalette;
    }
}

void ADayNightCycleManager::ApplyPalette(const FLight_SkyPalette& Palette)
{
    // Apply to Sun
    if (SunLight)
    {
        float ClampedPitch = FMath::Max(Palette.SunPitch, -30.0f);  // CAP guard
        SunLight->SetActorRotation(FRotator(ClampedPitch, Palette.SunYaw, 0.0f));
        UDirectionalLightComponent* DLC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (DLC)
        {
            DLC->SetIntensity(Palette.SunIntensity);
            DLC->SetLightColor(Palette.SunColor);
        }
    }

    // Apply to Fog
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FogComp)
        {
            FogComp->SetFogDensity(Palette.FogDensity);
            FogComp->SetFogInscatteringColor(Palette.FogColor);
        }
    }

    // Apply to SkyLight
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Palette.SkyLightIntensity);
            SLC->SetLightColor(Palette.SkyLightColor);
        }
    }
}

void ADayNightCycleManager::BlendPalettes(const FLight_SkyPalette& From, const FLight_SkyPalette& To, float Alpha)
{
    FLight_SkyPalette Blended;
    Blended.SunPitch        = FMath::Lerp(From.SunPitch, To.SunPitch, Alpha);
    Blended.SunYaw          = FMath::Lerp(From.SunYaw, To.SunYaw, Alpha);
    Blended.SunIntensity    = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    Blended.SunColor        = FLinearColor(
                                FMath::Lerp(From.SunColor.R, To.SunColor.R, Alpha),
                                FMath::Lerp(From.SunColor.G, To.SunColor.G, Alpha),
                                FMath::Lerp(From.SunColor.B, To.SunColor.B, Alpha), 1.0f);
    Blended.FogDensity      = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    Blended.FogColor        = FLinearColor(
                                FMath::Lerp(From.FogColor.R, To.FogColor.R, Alpha),
                                FMath::Lerp(From.FogColor.G, To.FogColor.G, Alpha),
                                FMath::Lerp(From.FogColor.B, To.FogColor.B, Alpha), 1.0f);
    Blended.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    Blended.SkyLightColor   = FLinearColor(
                                FMath::Lerp(From.SkyLightColor.R, To.SkyLightColor.R, Alpha),
                                FMath::Lerp(From.SkyLightColor.G, To.SkyLightColor.G, Alpha),
                                FMath::Lerp(From.SkyLightColor.B, To.SkyLightColor.B, Alpha), 1.0f);
    Blended.VolumetricFogScattering = FMath::Lerp(From.VolumetricFogScattering, To.VolumetricFogScattering, Alpha);

    ApplyPalette(Blended);
}

void ADayNightCycleManager::AutoFindSceneActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Auto-find DirectionalLight (Sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Auto-find ExponentialHeightFog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    // Auto-find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DayNightCycleManager: AutoFind — Sun=%s, Fog=%s, Sky=%s"),
        SunLight ? *SunLight->GetName() : TEXT("NONE"),
        HeightFog ? *HeightFog->GetName() : TEXT("NONE"),
        SkyLightActor ? *SkyLightActor->GetName() : TEXT("NONE"));
}

FString ADayNightCycleManager::GetPhaseDisplayName() const
{
    switch (GetCurrentPhase())
    {
        case ELight_DayPhase::Dawn:      return TEXT("Dawn");
        case ELight_DayPhase::Morning:   return TEXT("Morning");
        case ELight_DayPhase::Midday:    return TEXT("Midday");
        case ELight_DayPhase::Afternoon: return TEXT("Afternoon");
        case ELight_DayPhase::Dusk:      return TEXT("Dusk");
        case ELight_DayPhase::Twilight:  return TEXT("Twilight");
        case ELight_DayPhase::Night:     return TEXT("Night");
        case ELight_DayPhase::Midnight:  return TEXT("Midnight");
        default:                         return TEXT("Unknown");
    }
}

void ADayNightCycleManager::InitDefaultPalettes()
{
    // Dawn — peachy pink, low sun from east
    DawnPalette.SunPitch = -8.0f;
    DawnPalette.SunYaw = 85.0f;
    DawnPalette.SunIntensity = 4.5f;
    DawnPalette.SunColor = FLinearColor(1.0f, 0.72f, 0.52f, 1.0f);
    DawnPalette.FogDensity = 0.035f;
    DawnPalette.FogColor = FLinearColor(0.82f, 0.62f, 0.72f, 1.0f);
    DawnPalette.SkyLightIntensity = 1.2f;
    DawnPalette.SkyLightColor = FLinearColor(0.72f, 0.82f, 1.0f, 1.0f);
    DawnPalette.VolumetricFogScattering = 0.65f;

    // Morning — warm yellow, rising sun
    MorningPalette.SunPitch = -18.0f;
    MorningPalette.SunYaw = 120.0f;
    MorningPalette.SunIntensity = 6.5f;
    MorningPalette.SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);
    MorningPalette.FogDensity = 0.022f;
    MorningPalette.FogColor = FLinearColor(0.78f, 0.88f, 0.95f, 1.0f);
    MorningPalette.SkyLightIntensity = 1.5f;
    MorningPalette.SkyLightColor = FLinearColor(0.88f, 0.92f, 1.0f, 1.0f);
    MorningPalette.VolumetricFogScattering = 0.45f;

    // Midday — harsh white, overhead
    MiddayPalette.SunPitch = -75.0f;
    MiddayPalette.SunYaw = 180.0f;
    MiddayPalette.SunIntensity = 10.0f;
    MiddayPalette.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
    MiddayPalette.FogDensity = 0.012f;
    MiddayPalette.FogColor = FLinearColor(0.72f, 0.82f, 0.92f, 1.0f);
    MiddayPalette.SkyLightIntensity = 2.0f;
    MiddayPalette.SkyLightColor = FLinearColor(0.92f, 0.95f, 1.0f, 1.0f);
    MiddayPalette.VolumetricFogScattering = 0.28f;

    // Afternoon — golden warm
    AfternoonPalette.SunPitch = -25.0f;
    AfternoonPalette.SunYaw = 200.0f;
    AfternoonPalette.SunIntensity = 7.5f;
    AfternoonPalette.SunColor = FLinearColor(1.0f, 0.82f, 0.52f, 1.0f);
    AfternoonPalette.FogDensity = 0.022f;
    AfternoonPalette.FogColor = FLinearColor(0.85f, 0.75f, 0.58f, 1.0f);
    AfternoonPalette.SkyLightIntensity = 1.4f;
    AfternoonPalette.SkyLightColor = FLinearColor(1.0f, 0.88f, 0.72f, 1.0f);
    AfternoonPalette.VolumetricFogScattering = 0.42f;

    // Dusk — deep orange-red, setting sun
    DuskPalette.SunPitch = -12.0f;
    DuskPalette.SunYaw = 270.0f;
    DuskPalette.SunIntensity = 3.8f;
    DuskPalette.SunColor = FLinearColor(1.0f, 0.42f, 0.12f, 1.0f);
    DuskPalette.FogDensity = 0.028f;
    DuskPalette.FogColor = FLinearColor(0.85f, 0.38f, 0.12f, 1.0f);
    DuskPalette.SkyLightIntensity = 0.85f;
    DuskPalette.SkyLightColor = FLinearColor(1.0f, 0.72f, 0.45f, 1.0f);
    DuskPalette.VolumetricFogScattering = 0.55f;

    // Twilight — purple-blue transition
    TwilightPalette.SunPitch = -5.0f;
    TwilightPalette.SunYaw = 285.0f;
    TwilightPalette.SunIntensity = 1.2f;
    TwilightPalette.SunColor = FLinearColor(0.62f, 0.42f, 0.82f, 1.0f);
    TwilightPalette.FogDensity = 0.038f;
    TwilightPalette.FogColor = FLinearColor(0.28f, 0.22f, 0.52f, 1.0f);
    TwilightPalette.SkyLightIntensity = 0.45f;
    TwilightPalette.SkyLightColor = FLinearColor(0.52f, 0.55f, 0.82f, 1.0f);
    TwilightPalette.VolumetricFogScattering = 0.68f;

    // Night — deep blue, moonlight
    NightPalette.SunPitch = -30.0f;  // CAP minimum
    NightPalette.SunYaw = 0.0f;
    NightPalette.SunIntensity = 0.15f;
    NightPalette.SunColor = FLinearColor(0.38f, 0.45f, 0.72f, 1.0f);
    NightPalette.FogDensity = 0.045f;
    NightPalette.FogColor = FLinearColor(0.08f, 0.10f, 0.22f, 1.0f);
    NightPalette.SkyLightIntensity = 0.18f;
    NightPalette.SkyLightColor = FLinearColor(0.35f, 0.42f, 0.72f, 1.0f);
    NightPalette.VolumetricFogScattering = 0.82f;
}
