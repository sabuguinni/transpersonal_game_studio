#include "DawnAtmosphereSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ADawnAtmosphereSystem Implementation
// Agent #08 — Lighting & Atmosphere | Cycle AUTO_20260630_006
// Cretaceous Dawn/Sunrise — Cinematic prehistoric atmosphere
// ============================================================

ADawnAtmosphereSystem::ADawnAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for smooth transitions

    // Default dawn palette
    DawnPalette.SunPitchDegrees = -18.0f;
    DawnPalette.SunYawDegrees = 45.0f;
    DawnPalette.SunIntensity = 3.5f;
    DawnPalette.SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
    DawnPalette.FogDensity = 0.035f;
    DawnPalette.FogHeightFalloff = 0.2f;
    DawnPalette.FogInscatteringColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DawnPalette.SkylightIntensity = 1.2f;
    DawnPalette.bVolumetricFog = true;
    DawnPalette.VolumetricFogScatteringDistribution = 0.2f;

    AtmosphereState.CurrentTimeOfDay = ELight_TimeOfDay::Dawn;
    AtmosphereState.CurrentSunAngle = -18.0f;
    AtmosphereState.CurrentFogDensity = 0.035f;
    AtmosphereState.bIsTransitioning = false;
    AtmosphereState.TransitionProgress = 0.0f;
}

void ADawnAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();

    // Auto-find light actors if not manually assigned
    AutoFindLightActors();

    // Apply initial palette
    ApplyDawnPalette();

    // Apply Lumen settings
    ApplyLumenSettings();
}

void ADawnAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // === TRANSITION SYSTEM ===
    if (bIsTransitioning)
    {
        TransitionElapsed += DeltaTime;
        float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);

        // Smooth step for cinematic feel
        float SmoothedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

        FLight_DawnPalette CurrentPalette = LerpPalettes(PaletteFrom, PaletteTo, SmoothedAlpha);
        ApplyDirectionalLight(CurrentPalette);
        ApplyFog(CurrentPalette);
        ApplySkyLight(CurrentPalette);

        AtmosphereState.TransitionProgress = Alpha;
        AtmosphereState.CurrentSunAngle = CurrentPalette.SunPitchDegrees;
        AtmosphereState.CurrentFogDensity = CurrentPalette.FogDensity;

        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            AtmosphereState.CurrentTimeOfDay = TransitionTo;
            AtmosphereState.bIsTransitioning = false;
            AtmosphereState.TransitionProgress = 1.0f;
            DawnPalette = PaletteTo;
        }
    }

    // === AUTO DAY/NIGHT CYCLE ===
    if (bAutoCycleDayNight && !bIsTransitioning)
    {
        DayCycleElapsed += DeltaTime;
        if (DayCycleElapsed >= FullDayCycleDuration)
        {
            DayCycleElapsed = 0.0f;
        }

        // Cycle through time of day based on elapsed time
        float CycleProgress = DayCycleElapsed / FullDayCycleDuration;
        ELight_TimeOfDay NewTimeOfDay;

        if (CycleProgress < 0.15f)
            NewTimeOfDay = ELight_TimeOfDay::Dawn;
        else if (CycleProgress < 0.45f)
            NewTimeOfDay = ELight_TimeOfDay::Midday;
        else if (CycleProgress < 0.65f)
            NewTimeOfDay = ELight_TimeOfDay::Dusk;
        else
            NewTimeOfDay = ELight_TimeOfDay::Night;

        if (NewTimeOfDay != AtmosphereState.CurrentTimeOfDay)
        {
            TransitionToTimeOfDay(NewTimeOfDay);
        }
    }
}

void ADawnAtmosphereSystem::ApplyDawnPalette()
{
    ApplyDirectionalLight(DawnPalette);
    ApplyFog(DawnPalette);
    ApplySkyLight(DawnPalette);

    AtmosphereState.CurrentTimeOfDay = TargetTimeOfDay;
    AtmosphereState.CurrentSunAngle = DawnPalette.SunPitchDegrees;
    AtmosphereState.CurrentFogDensity = DawnPalette.FogDensity;

    UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Dawn palette applied — Sun pitch %.1f, Fog density %.3f"),
        DawnPalette.SunPitchDegrees, DawnPalette.FogDensity);
}

void ADawnAtmosphereSystem::TransitionToTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    if (bIsTransitioning) return;

    TransitionFrom = AtmosphereState.CurrentTimeOfDay;
    TransitionTo = NewTimeOfDay;
    PaletteFrom = DawnPalette;
    PaletteTo = BuildPaletteForTimeOfDay(NewTimeOfDay);

    bIsTransitioning = true;
    TransitionElapsed = 0.0f;
    AtmosphereState.bIsTransitioning = true;
    AtmosphereState.TransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Transitioning from %d to %d over %.1f seconds"),
        (int32)TransitionFrom, (int32)TransitionTo, TransitionDuration);
}

void ADawnAtmosphereSystem::AutoFindLightActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    if (!DirectionalLightActor)
    {
        for (TActorIterator<ADirectionalLight> It(World); It; ++It)
        {
            DirectionalLightActor = *It;
            UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Found DirectionalLight: %s"), *DirectionalLightActor->GetName());
            break;
        }
    }

    // Find SkyLight
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = FoundActors[0];
            UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Found SkyLight: %s"), *SkyLightActor->GetName());
        }
    }

    // Find ExponentialHeightFog
    if (!FogActor)
    {
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            FogActor = FoundFog[0];
            UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Found ExponentialHeightFog: %s"), *FogActor->GetName());
        }
    }
}

void ADawnAtmosphereSystem::ApplyLumenSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (bEnableLumenGI)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.GlobalIllumination.Allow 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    }

    if (bEnableLumenReflections)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections.Allow 1"));
    }

    if (DawnPalette.bVolumetricFog)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog 1"));
    }

    if (bEnableVolumetricClouds)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricCloud 1"));
    }

    UE_LOG(LogTemp, Log, TEXT("DawnAtmosphereSystem: Lumen settings applied"));
}

FLinearColor ADawnAtmosphereSystem::GetSunColorForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        return FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);   // Warm orange-pink
    case ELight_TimeOfDay::Midday:
        return FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);   // Bright white-yellow
    case ELight_TimeOfDay::Dusk:
        return FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);   // Deep orange-red
    case ELight_TimeOfDay::Night:
        return FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);     // Cool blue moonlight
    case ELight_TimeOfDay::Overcast:
        return FLinearColor(0.75f, 0.78f, 0.85f, 1.0f);  // Grey-blue overcast
    default:
        return FLinearColor::White;
    }
}

float ADawnAtmosphereSystem::GetFogDensityForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:    return 0.035f;   // Morning mist
    case ELight_TimeOfDay::Midday:  return 0.008f;   // Clear midday
    case ELight_TimeOfDay::Dusk:    return 0.025f;   // Evening haze
    case ELight_TimeOfDay::Night:   return 0.045f;   // Night mist
    case ELight_TimeOfDay::Overcast: return 0.06f;   // Heavy overcast fog
    default:                        return 0.01f;
    }
}

// === PRIVATE IMPLEMENTATIONS ===

void ADawnAtmosphereSystem::ApplyDirectionalLight(const FLight_DawnPalette& Palette)
{
    if (!DirectionalLightActor) return;

    // CAP GUARD: Never set sun pitch above -15 degrees (would go below horizon)
    float SafePitch = FMath::Min(Palette.SunPitchDegrees, -15.0f);

    DirectionalLightActor->SetActorRotation(
        FRotator(SafePitch, Palette.SunYawDegrees, 0.0f)
    );

    UDirectionalLightComponent* LightComp = DirectionalLightActor->GetComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(Palette.SunIntensity);
        LightComp->SetLightColor(Palette.SunColor.ToFColor(true));
        LightComp->SetAtmosphereSunLight(true);
        LightComp->SetCastShadows(true);
    }
}

void ADawnAtmosphereSystem::ApplyFog(const FLight_DawnPalette& Palette)
{
    if (!FogActor) return;

    UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;

    FogComp->SetFogDensity(Palette.FogDensity);
    FogComp->SetFogHeightFalloff(Palette.FogHeightFalloff);
    FogComp->SetFogInscatteringColor(Palette.FogInscatteringColor.ToFColor(true));
    FogComp->SetVolumetricFog(Palette.bVolumetricFog);
    FogComp->SetVolumetricFogScatteringDistribution(Palette.VolumetricFogScatteringDistribution);
    FogComp->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.85f, 0.75f, 1.0f).ToFColor(true));
}

void ADawnAtmosphereSystem::ApplySkyLight(const FLight_DawnPalette& Palette)
{
    if (!SkyLightActor) return;

    USkyLightComponent* SkyComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (!SkyComp) return;

    SkyComp->SetIntensity(Palette.SkylightIntensity);
    SkyComp->SetRealTimeCapture(true);
    SkyComp->SetLowerHemisphereIsBlack(false);
}

FLight_DawnPalette ADawnAtmosphereSystem::BuildPaletteForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    FLight_DawnPalette Palette;

    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        Palette.SunPitchDegrees = -18.0f;
        Palette.SunYawDegrees = 45.0f;
        Palette.SunIntensity = 3.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);
        Palette.FogDensity = 0.035f;
        Palette.FogInscatteringColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
        Palette.SkylightIntensity = 1.2f;
        break;

    case ELight_TimeOfDay::Midday:
        Palette.SunPitchDegrees = -75.0f;
        Palette.SunYawDegrees = 0.0f;
        Palette.SunIntensity = 10.0f;
        Palette.SunColor = FLinearColor(1.0f, 0.98f, 0.92f, 1.0f);
        Palette.FogDensity = 0.008f;
        Palette.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        Palette.SkylightIntensity = 2.0f;
        break;

    case ELight_TimeOfDay::Dusk:
        Palette.SunPitchDegrees = -15.0f;
        Palette.SunYawDegrees = -135.0f;
        Palette.SunIntensity = 2.5f;
        Palette.SunColor = FLinearColor(1.0f, 0.55f, 0.25f, 1.0f);
        Palette.FogDensity = 0.025f;
        Palette.FogInscatteringColor = FLinearColor(0.85f, 0.45f, 0.25f, 1.0f);
        Palette.SkylightIntensity = 0.8f;
        break;

    case ELight_TimeOfDay::Night:
        Palette.SunPitchDegrees = -20.0f;
        Palette.SunYawDegrees = 180.0f;
        Palette.SunIntensity = 0.5f;
        Palette.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
        Palette.FogDensity = 0.045f;
        Palette.FogInscatteringColor = FLinearColor(0.15f, 0.2f, 0.4f, 1.0f);
        Palette.SkylightIntensity = 0.3f;
        break;

    case ELight_TimeOfDay::Overcast:
        Palette.SunPitchDegrees = -45.0f;
        Palette.SunYawDegrees = 0.0f;
        Palette.SunIntensity = 4.0f;
        Palette.SunColor = FLinearColor(0.75f, 0.78f, 0.85f, 1.0f);
        Palette.FogDensity = 0.06f;
        Palette.FogInscatteringColor = FLinearColor(0.6f, 0.65f, 0.75f, 1.0f);
        Palette.SkylightIntensity = 1.5f;
        break;

    default:
        break;
    }

    return Palette;
}

FLight_DawnPalette ADawnAtmosphereSystem::LerpPalettes(const FLight_DawnPalette& A, const FLight_DawnPalette& B, float Alpha) const
{
    FLight_DawnPalette Result;

    Result.SunPitchDegrees = FMath::Lerp(A.SunPitchDegrees, B.SunPitchDegrees, Alpha);
    Result.SunYawDegrees = FMath::Lerp(A.SunYawDegrees, B.SunYawDegrees, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.SkylightIntensity = FMath::Lerp(A.SkylightIntensity, B.SkylightIntensity, Alpha);
    Result.VolumetricFogScatteringDistribution = FMath::Lerp(A.VolumetricFogScatteringDistribution, B.VolumetricFogScatteringDistribution, Alpha);
    Result.bVolumetricFog = Alpha < 0.5f ? A.bVolumetricFog : B.bVolumetricFog;

    return Result;
}

#if WITH_EDITOR
void ADawnAtmosphereSystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Live preview in editor — apply palette when any property changes
    if (PropertyChangedEvent.Property)
    {
        AutoFindLightActors();
        ApplyDawnPalette();
    }
}
#endif
