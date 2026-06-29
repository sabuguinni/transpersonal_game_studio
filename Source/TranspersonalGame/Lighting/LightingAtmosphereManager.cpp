#include "LightingAtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// ALightingAtmosphereManager — Implementation
// Director of Photography for the Cretaceous world.
// Six cinematic lighting states, each with a distinct emotional palette.
// ─────────────────────────────────────────────────────────────────────────────

ALightingAtmosphereManager::ALightingAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick — sufficient for lighting transitions

    // Initialise default palette data in constructor
    InitialiseDefaultPalettes();
}

void ALightingAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();

    // Apply the current time-of-day palette immediately on game start
    ApplyTimeOfDay(CurrentTimeOfDay);
}

void ALightingAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Auto cycle advancement ────────────────────────────────────────────────
    if (bAutoCycle && DayCycleDurationSeconds > 0.0f)
    {
        CurrentCycleTime += DeltaTime;
        if (CurrentCycleTime >= DayCycleDurationSeconds)
        {
            CurrentCycleTime = 0.0f;
        }

        // Map cycle time to time-of-day state (6 equal segments)
        float NormalizedTime = CurrentCycleTime / DayCycleDurationSeconds;
        ELight_TimeOfDay NewState;

        if      (NormalizedTime < 0.1f)  NewState = ELight_TimeOfDay::Dawn;
        else if (NormalizedTime < 0.25f) NewState = ELight_TimeOfDay::Morning;
        else if (NormalizedTime < 0.55f) NewState = ELight_TimeOfDay::Midday;
        else if (NormalizedTime < 0.70f) NewState = ELight_TimeOfDay::Afternoon;
        else if (NormalizedTime < 0.85f) NewState = ELight_TimeOfDay::Dusk;
        else                             NewState = ELight_TimeOfDay::Night;

        if (NewState != CurrentTimeOfDay && !bIsTransitioning)
        {
            TransitionToTimeOfDay(NewState, TransitionDuration);
        }
    }

    // ── Transition blending ───────────────────────────────────────────────────
    if (bIsTransitioning)
    {
        TransitionElapsed += DeltaTime;
        float Alpha = FMath::Clamp(TransitionElapsed / ActiveTransitionDuration, 0.0f, 1.0f);
        Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha); // Ease in/out

        // Blend between start palette and target palette
        const FLight_SunPalette* TargetPalettePtr = Palettes.Find(TransitionTarget);
        if (TargetPalettePtr)
        {
            FLight_SunPalette BlendedPalette;
            BlendedPalette.SunColour          = FLinearColor::LerpUsingHSV(TransitionStartPalette.SunColour, TargetPalettePtr->SunColour, Alpha);
            BlendedPalette.SunIntensity       = FMath::Lerp(TransitionStartPalette.SunIntensity, TargetPalettePtr->SunIntensity, Alpha);
            BlendedPalette.SunPitchDegrees    = FMath::Lerp(TransitionStartPalette.SunPitchDegrees, TargetPalettePtr->SunPitchDegrees, Alpha);
            BlendedPalette.FogInscatterColour = FLinearColor::LerpUsingHSV(TransitionStartPalette.FogInscatterColour, TargetPalettePtr->FogInscatterColour, Alpha);
            BlendedPalette.FogDensity         = FMath::Lerp(TransitionStartPalette.FogDensity, TargetPalettePtr->FogDensity, Alpha);
            BlendedPalette.SkyLightIntensity  = FMath::Lerp(TransitionStartPalette.SkyLightIntensity, TargetPalettePtr->SkyLightIntensity, Alpha);
            BlendedPalette.AmbientColour      = FLinearColor::LerpUsingHSV(TransitionStartPalette.AmbientColour, TargetPalettePtr->AmbientColour, Alpha);

            ApplySunPalette(BlendedPalette);
            ApplyFogPalette(BlendedPalette);
            ApplySkyLightPalette(BlendedPalette);
        }

        if (Alpha >= 1.0f)
        {
            // Transition complete
            CurrentTimeOfDay = TransitionTarget;
            bIsTransitioning = false;
            TransitionElapsed = 0.0f;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ALightingAtmosphereManager::ApplyTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    bIsTransitioning = false;

    const FLight_SunPalette* PalettePtr = Palettes.Find(NewTimeOfDay);
    if (!PalettePtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("LightingAtmosphereManager: No palette found for state %d"), (int32)NewTimeOfDay);
        return;
    }

    ApplySunPalette(*PalettePtr);
    ApplyFogPalette(*PalettePtr);
    ApplySkyLightPalette(*PalettePtr);
}

void ALightingAtmosphereManager::TransitionToTimeOfDay(ELight_TimeOfDay TargetState, float BlendDuration)
{
    if (bIsTransitioning && TransitionTarget == TargetState)
    {
        return; // Already transitioning to this state
    }

    // Capture current palette as transition start
    const FLight_SunPalette* CurrentPalettePtr = Palettes.Find(CurrentTimeOfDay);
    if (CurrentPalettePtr)
    {
        TransitionStartPalette = *CurrentPalettePtr;
    }

    TransitionTarget = TargetState;
    ActiveTransitionDuration = FMath::Max(BlendDuration, 0.1f);
    TransitionElapsed = 0.0f;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: Transitioning to state %d over %.1f seconds"),
           (int32)TargetState, BlendDuration);
}

FLight_SunPalette ALightingAtmosphereManager::GetPaletteForState(ELight_TimeOfDay State) const
{
    const FLight_SunPalette* PalettePtr = Palettes.Find(State);
    if (PalettePtr)
    {
        return *PalettePtr;
    }
    return FLight_SunPalette(); // Return default if not found
}

void ALightingAtmosphereManager::RefreshSceneLighting()
{
    ApplyTimeOfDay(CurrentTimeOfDay);
    UE_LOG(LogTemp, Log, TEXT("LightingAtmosphereManager: Scene lighting refreshed for state %d"), (int32)CurrentTimeOfDay);
}

void ALightingAtmosphereManager::InitialiseDefaultPalettes()
{
    Palettes.Empty();

    // ── DAWN — golden hour, long east shadows, warm orange-pink ──────────────
    FLight_SunPalette DawnPalette;
    DawnPalette.SunColour          = FLinearColor(1.0f, 0.75f, 0.40f, 1.0f);
    DawnPalette.SunIntensity       = 4.0f;
    DawnPalette.SunPitchDegrees    = 8.0f;   // Just above horizon
    DawnPalette.FogInscatterColour = FLinearColor(0.85f, 0.60f, 0.45f, 1.0f);
    DawnPalette.FogDensity         = 0.04f;
    DawnPalette.SkyLightIntensity  = 0.6f;
    DawnPalette.AmbientColour      = FLinearColor(0.70f, 0.55f, 0.65f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Dawn, DawnPalette);

    // ── MORNING — warm fill, soft contrast, clear sky ─────────────────────────
    FLight_SunPalette MorningPalette;
    MorningPalette.SunColour          = FLinearColor(1.0f, 0.92f, 0.72f, 1.0f);
    MorningPalette.SunIntensity       = 7.0f;
    MorningPalette.SunPitchDegrees    = -30.0f;
    MorningPalette.FogInscatterColour = FLinearColor(0.75f, 0.82f, 0.95f, 1.0f);
    MorningPalette.FogDensity         = 0.025f;
    MorningPalette.SkyLightIntensity  = 0.85f;
    MorningPalette.AmbientColour      = FLinearColor(0.65f, 0.72f, 0.88f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Morning, MorningPalette);

    // ── MIDDAY — harsh overhead, bright whites, deep cool shadows ─────────────
    FLight_SunPalette MiddayPalette;
    MiddayPalette.SunColour          = FLinearColor(1.0f, 0.98f, 0.90f, 1.0f);
    MiddayPalette.SunIntensity       = 12.0f;
    MiddayPalette.SunPitchDegrees    = -75.0f;  // Nearly overhead
    MiddayPalette.FogInscatterColour = FLinearColor(0.60f, 0.70f, 0.90f, 1.0f);
    MiddayPalette.FogDensity         = 0.015f;
    MiddayPalette.SkyLightIntensity  = 1.2f;
    MiddayPalette.AmbientColour      = FLinearColor(0.55f, 0.65f, 0.85f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Midday, MiddayPalette);

    // ── AFTERNOON — warm amber, medium angle, long shadows west ──────────────
    FLight_SunPalette AfternoonPalette;
    AfternoonPalette.SunColour          = FLinearColor(1.0f, 0.85f, 0.55f, 1.0f);
    AfternoonPalette.SunIntensity       = 9.0f;
    AfternoonPalette.SunPitchDegrees    = -40.0f;
    AfternoonPalette.FogInscatterColour = FLinearColor(0.80f, 0.72f, 0.60f, 1.0f);
    AfternoonPalette.FogDensity         = 0.022f;
    AfternoonPalette.SkyLightIntensity  = 0.95f;
    AfternoonPalette.AmbientColour      = FLinearColor(0.72f, 0.65f, 0.55f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Afternoon, AfternoonPalette);

    // ── DUSK — orange-purple twilight, dramatic silhouettes ──────────────────
    FLight_SunPalette DuskPalette;
    DuskPalette.SunColour          = FLinearColor(1.0f, 0.45f, 0.15f, 1.0f);
    DuskPalette.SunIntensity       = 3.0f;
    DuskPalette.SunPitchDegrees    = 5.0f;   // Just below horizon
    DuskPalette.FogInscatterColour = FLinearColor(0.70f, 0.40f, 0.55f, 1.0f);
    DuskPalette.FogDensity         = 0.05f;
    DuskPalette.SkyLightIntensity  = 0.4f;
    DuskPalette.AmbientColour      = FLinearColor(0.45f, 0.35f, 0.55f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Dusk, DuskPalette);

    // ── NIGHT — moonlight, deep indigo shadows, stars ─────────────────────────
    FLight_SunPalette NightPalette;
    NightPalette.SunColour          = FLinearColor(0.70f, 0.80f, 1.0f, 1.0f);
    NightPalette.SunIntensity       = 1.5f;
    NightPalette.SunPitchDegrees    = -15.0f;  // Moon angle
    NightPalette.FogInscatterColour = FLinearColor(0.10f, 0.12f, 0.25f, 1.0f);
    NightPalette.FogDensity         = 0.06f;
    NightPalette.SkyLightIntensity  = 0.2f;
    NightPalette.AmbientColour      = FLinearColor(0.15f, 0.18f, 0.35f, 1.0f);
    Palettes.Add(ELight_TimeOfDay::Night, NightPalette);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void ALightingAtmosphereManager::ApplySunPalette(const FLight_SunPalette& Palette)
{
    if (!SunActor)
    {
        return;
    }

    // Set rotation (pitch = sun elevation angle)
    FRotator CurrentRot = SunActor->GetActorRotation();
    SunActor->SetActorRotation(FRotator(Palette.SunPitchDegrees, CurrentRot.Yaw, CurrentRot.Roll));

    // Apply colour and intensity to directional light component
    UDirectionalLightComponent* DLC = SunActor->FindComponentByClass<UDirectionalLightComponent>();
    if (DLC)
    {
        DLC->SetIntensity(Palette.SunIntensity);
        DLC->SetLightColor(Palette.SunColour);
    }
}

void ALightingAtmosphereManager::ApplyFogPalette(const FLight_SunPalette& Palette)
{
    if (!FogActor)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComp = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (FogComp)
    {
        FogComp->SetFogDensity(Palette.FogDensity);
        FogComp->SetFogInscatteringColor(Palette.FogInscatterColour);
    }
}

void ALightingAtmosphereManager::ApplySkyLightPalette(const FLight_SunPalette& Palette)
{
    if (!SkyLightActor)
    {
        return;
    }

    USkyLightComponent* SLC = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (SLC)
    {
        SLC->SetIntensity(Palette.SkyLightIntensity);
    }
}
