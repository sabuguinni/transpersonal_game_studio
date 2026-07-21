#include "DuskAtmosphereController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ADuskAtmosphereController::ADuskAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz is enough for lighting transitions

    InitDefaultPresets();
}

void ADuskAtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    AutoFindLightingActors();
    ApplyPaletteImmediate(CurrentPalette);
}

void ADuskAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto cycle: advance time and blend palettes
    if (bAutoCycle && FullDayDurationSeconds > 0.0f)
    {
        CycleElapsedSeconds += DeltaTime;
        if (CycleElapsedSeconds >= FullDayDurationSeconds)
        {
            CycleElapsedSeconds = 0.0f;
        }
    }

    // Blend transition
    if (bTransitioning)
    {
        BlendAlpha = FMath::Clamp(BlendAlpha + TransitionSpeed * DeltaTime, 0.0f, 1.0f);

        FLight_PaletteConfig From = GetPaletteConfig(CurrentPalette);
        FLight_PaletteConfig To = GetPaletteConfig(TargetPalette);
        FLight_PaletteConfig Blended = BlendConfigs(From, To, BlendAlpha);
        ApplyConfig(Blended);

        if (BlendAlpha >= 1.0f)
        {
            CurrentPalette = TargetPalette;
            BlendAlpha = 0.0f;
            bTransitioning = false;
        }
    }
}

void ADuskAtmosphereController::AutoFindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight (sun)
    if (!SunLight)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            SunLight = *It;
            break;
        }
    }

    // Find ExponentialHeightFog
    if (!HeightFog)
    {
        for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
        {
            HeightFog = *It;
            break;
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        for (TActorIterator<ASkyLight> It(World); It; ++It)
        {
            SkyLightActor = *It;
            break;
        }
    }
}

void ADuskAtmosphereController::ApplyPaletteImmediate(ELight_DayPalette Palette)
{
    CurrentPalette = Palette;
    bTransitioning = false;
    BlendAlpha = 0.0f;
    ApplyConfig(GetPaletteConfig(Palette));
}

void ADuskAtmosphereController::BeginTransitionTo(ELight_DayPalette NewTarget, float Speed)
{
    if (NewTarget == CurrentPalette) return;
    TargetPalette = NewTarget;
    TransitionSpeed = FMath::Max(Speed, 0.001f);
    BlendAlpha = 0.0f;
    bTransitioning = true;
}

FLight_PaletteConfig ADuskAtmosphereController::GetPaletteConfig(ELight_DayPalette Palette) const
{
    switch (Palette)
    {
        case ELight_DayPalette::Dawn:     return DawnPreset;
        case ELight_DayPalette::Midday:   return MiddayPreset;
        case ELight_DayPalette::Dusk:     return DuskPreset;
        case ELight_DayPalette::Night:    return NightPreset;
        case ELight_DayPalette::Overcast: return OvercastPreset;
        case ELight_DayPalette::Volcanic: return VolcanicPreset;
        default:                          return DuskPreset;
    }
}

float ADuskAtmosphereController::GetTimeOfDayNormalized() const
{
    if (FullDayDurationSeconds <= 0.0f) return 0.5f;
    return CycleElapsedSeconds / FullDayDurationSeconds;
}

void ADuskAtmosphereController::ApplyConfig(const FLight_PaletteConfig& Config)
{
    // Apply sun settings
    if (SunLight)
    {
        SunLight->SetActorRotation(FRotator(Config.SunPitch, Config.SunYaw, 0.0f));
        UDirectionalLightComponent* LC = SunLight->GetComponentByClass<UDirectionalLightComponent>();
        if (LC)
        {
            LC->SetIntensity(Config.SunIntensity);
            LC->SetLightColor(Config.SunColor);
        }
    }

    // Apply fog settings
    if (HeightFog)
    {
        UExponentialHeightFogComponent* FC = HeightFog->GetComponentByClass<UExponentialHeightFogComponent>();
        if (FC)
        {
            FC->SetFogDensity(Config.FogDensity);
            FC->SetFogInscatteringColor(Config.FogColor);
            FC->SetVolumetricFog(Config.bVolumetricFog);
        }
    }

    // Apply sky light settings
    if (SkyLightActor)
    {
        USkyLightComponent* SLC = SkyLightActor->GetComponentByClass<USkyLightComponent>();
        if (SLC)
        {
            SLC->SetIntensity(Config.SkyLightIntensity);
            SLC->SetLightColor(Config.SkyLightColor);
        }
    }
}

FLight_PaletteConfig ADuskAtmosphereController::BlendConfigs(
    const FLight_PaletteConfig& A,
    const FLight_PaletteConfig& B,
    float Alpha) const
{
    FLight_PaletteConfig Result;
    Result.SunPitch         = FMath::Lerp(A.SunPitch, B.SunPitch, Alpha);
    Result.SunYaw           = FMath::Lerp(A.SunYaw, B.SunYaw, Alpha);
    Result.SunIntensity     = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor         = FLinearColor(
        FMath::Lerp(A.SunColor.R, B.SunColor.R, Alpha),
        FMath::Lerp(A.SunColor.G, B.SunColor.G, Alpha),
        FMath::Lerp(A.SunColor.B, B.SunColor.B, Alpha),
        1.0f);
    Result.FogDensity       = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor         = FLinearColor(
        FMath::Lerp(A.FogColor.R, B.FogColor.R, Alpha),
        FMath::Lerp(A.FogColor.G, B.FogColor.G, Alpha),
        FMath::Lerp(A.FogColor.B, B.FogColor.B, Alpha),
        1.0f);
    Result.bVolumetricFog   = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.SkyLightColor    = FLinearColor(
        FMath::Lerp(A.SkyLightColor.R, B.SkyLightColor.R, Alpha),
        FMath::Lerp(A.SkyLightColor.G, B.SkyLightColor.G, Alpha),
        FMath::Lerp(A.SkyLightColor.B, B.SkyLightColor.B, Alpha),
        1.0f);
    Result.MieAnisotropy    = FMath::Lerp(A.MieAnisotropy, B.MieAnisotropy, Alpha);
    Result.RayleighScale    = FMath::Lerp(A.RayleighScale, B.RayleighScale, Alpha);
    return Result;
}

void ADuskAtmosphereController::InitDefaultPresets()
{
    // === DAWN — Hope, cautious optimism, first light ===
    DawnPreset.SunPitch         = -8.0f;
    DawnPreset.SunYaw           = -90.0f;
    DawnPreset.SunIntensity     = 3.5f;
    DawnPreset.SunColor         = FLinearColor(1.0f, 0.72f, 0.38f, 1.0f);
    DawnPreset.FogDensity       = 0.045f;
    DawnPreset.FogColor         = FLinearColor(0.78f, 0.55f, 0.35f, 1.0f);
    DawnPreset.bVolumetricFog   = true;
    DawnPreset.SkyLightIntensity = 1.2f;
    DawnPreset.SkyLightColor    = FLinearColor(0.9f, 0.75f, 0.6f, 1.0f);
    DawnPreset.MieAnisotropy    = 0.78f;
    DawnPreset.RayleighScale    = 0.0331f;

    // === MIDDAY — Exposure, vulnerability, harsh reality ===
    MiddayPreset.SunPitch         = -75.0f;
    MiddayPreset.SunYaw           = 0.0f;
    MiddayPreset.SunIntensity     = 12.0f;
    MiddayPreset.SunColor         = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);
    MiddayPreset.FogDensity       = 0.008f;
    MiddayPreset.FogColor         = FLinearColor(0.65f, 0.78f, 0.95f, 1.0f);
    MiddayPreset.bVolumetricFog   = false;
    MiddayPreset.SkyLightIntensity = 2.5f;
    MiddayPreset.SkyLightColor    = FLinearColor(0.88f, 0.92f, 1.0f, 1.0f);
    MiddayPreset.MieAnisotropy    = 0.65f;
    MiddayPreset.RayleighScale    = 0.0331f;

    // === DUSK — Urgency, beauty before darkness, last chance ===
    DuskPreset.SunPitch         = -18.0f;
    DuskPreset.SunYaw           = 55.0f;
    DuskPreset.SunIntensity     = 6.5f;
    DuskPreset.SunColor         = FLinearColor(1.0f, 0.55f, 0.18f, 1.0f);
    DuskPreset.FogDensity       = 0.028f;
    DuskPreset.FogColor         = FLinearColor(0.85f, 0.42f, 0.12f, 1.0f);
    DuskPreset.bVolumetricFog   = true;
    DuskPreset.SkyLightIntensity = 1.8f;
    DuskPreset.SkyLightColor    = FLinearColor(0.9f, 0.65f, 0.45f, 1.0f);
    DuskPreset.MieAnisotropy    = 0.82f;
    DuskPreset.RayleighScale    = 0.0331f;

    // === NIGHT — Fear, stealth, primal danger ===
    NightPreset.SunPitch         = -15.0f;
    NightPreset.SunYaw           = 200.0f;
    NightPreset.SunIntensity     = 0.8f;
    NightPreset.SunColor         = FLinearColor(0.55f, 0.65f, 0.9f, 1.0f);
    NightPreset.FogDensity       = 0.055f;
    NightPreset.FogColor         = FLinearColor(0.08f, 0.1f, 0.22f, 1.0f);
    NightPreset.bVolumetricFog   = true;
    NightPreset.SkyLightIntensity = 0.4f;
    NightPreset.SkyLightColor    = FLinearColor(0.45f, 0.5f, 0.75f, 1.0f);
    NightPreset.MieAnisotropy    = 0.55f;
    NightPreset.RayleighScale    = 0.0165f;

    // === OVERCAST — Dread, storm survival, oppressive weight ===
    OvercastPreset.SunPitch         = -35.0f;
    OvercastPreset.SunYaw           = 0.0f;
    OvercastPreset.SunIntensity     = 2.0f;
    OvercastPreset.SunColor         = FLinearColor(0.72f, 0.72f, 0.75f, 1.0f);
    OvercastPreset.FogDensity       = 0.065f;
    OvercastPreset.FogColor         = FLinearColor(0.45f, 0.48f, 0.52f, 1.0f);
    OvercastPreset.bVolumetricFog   = true;
    OvercastPreset.SkyLightIntensity = 1.0f;
    OvercastPreset.SkyLightColor    = FLinearColor(0.6f, 0.62f, 0.68f, 1.0f);
    OvercastPreset.MieAnisotropy    = 0.45f;
    OvercastPreset.RayleighScale    = 0.0165f;

    // === VOLCANIC — Apocalyptic, awe, existential threat ===
    VolcanicPreset.SunPitch         = -25.0f;
    VolcanicPreset.SunYaw           = 30.0f;
    VolcanicPreset.SunIntensity     = 4.0f;
    VolcanicPreset.SunColor         = FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
    VolcanicPreset.FogDensity       = 0.085f;
    VolcanicPreset.FogColor         = FLinearColor(0.65f, 0.22f, 0.05f, 1.0f);
    VolcanicPreset.bVolumetricFog   = true;
    VolcanicPreset.SkyLightIntensity = 0.9f;
    VolcanicPreset.SkyLightColor    = FLinearColor(0.8f, 0.4f, 0.15f, 1.0f);
    VolcanicPreset.MieAnisotropy    = 0.88f;
    VolcanicPreset.RayleighScale    = 0.0498f;
}
