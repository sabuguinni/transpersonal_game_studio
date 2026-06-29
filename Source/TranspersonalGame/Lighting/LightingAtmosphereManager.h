#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "LightingAtmosphereManager.generated.h"

/**
 * ELight_TimeOfDay — The six cinematic lighting states for the prehistoric world.
 * Each state has a distinct emotional palette inspired by Roger Deakins' work.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),        // Cycle 006: golden hour, long shadows east
    Morning     UMETA(DisplayName = "Morning"),     // Cycle 007: warm fill, soft contrast
    Midday      UMETA(DisplayName = "Midday"),      // Cycle 008: harsh overhead, deep shadows
    Afternoon   UMETA(DisplayName = "Afternoon"),   // Cycle 009: warm amber, medium angle
    Dusk        UMETA(DisplayName = "Dusk"),        // Cycle 005: orange-purple twilight
    Night       UMETA(DisplayName = "Night"),       // Cycle 006: moonlight, indigo shadows
};

/**
 * FLight_SunPalette — Colour and intensity data for a single time-of-day state.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_SunPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColour = FLinearColor(1.0f, 0.98f, 0.90f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchDegrees = -75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogInscatterColour = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor AmbientColour = FLinearColor(0.55f, 0.65f, 0.85f, 1.0f);
};

/**
 * FLight_FillLightConfig — Configuration for a single cinematic fill light.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_FillLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    FString LightName = TEXT("FillLight_Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    FVector RelativeLocation = FVector(200.0f, 0.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    FLinearColor LightColour = FLinearColor(1.0f, 0.97f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    float Intensity = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    float AttenuationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLight")
    bool bCastShadows = true;
};

/**
 * ALightingAtmosphereManager — Runtime manager for the prehistoric world's dynamic lighting.
 *
 * Manages the full day/night cycle across 6 cinematic time-of-day states.
 * Each state uses a curated palette (sun colour, intensity, pitch, fog, fill lights)
 * to create emotionally distinct lighting conditions.
 *
 * Integrates with:
 *   - UDirectionalLightComponent (sun/moon)
 *   - UExponentialHeightFogComponent (atmosphere)
 *   - USkyLightComponent (ambient GI)
 *   - UE5 Lumen (real-time GI + reflections)
 *   - Post-process volumes (colour grading per time-of-day)
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    // ── Current State ─────────────────────────────────────────────────────────

    /** Current time-of-day lighting state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    /** How long (seconds) a full day cycle takes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float DayCycleDurationSeconds = 600.0f;

    /** Whether the day/night cycle auto-advances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bAutoCycle = false;

    /** Transition blend time between states (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float TransitionDuration = 8.0f;

    // ── Palette Data ──────────────────────────────────────────────────────────

    /** Palette for each time-of-day state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    TMap<ELight_TimeOfDay, FLight_SunPalette> Palettes;

    /** Fill light configurations for current state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|FillLights")
    TArray<FLight_FillLightConfig> FillLightConfigs;

    // ── Scene References ──────────────────────────────────────────────────────

    /** Reference to the scene's directional light (sun/moon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SunActor;

    /** Reference to the scene's sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyLightActor;

    /** Reference to the scene's fog actor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> FogActor;

    // ── Runtime Stats ─────────────────────────────────────────────────────────

    /** Current elapsed time in the day cycle (seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State",
              meta = (AllowPrivateAccess = "true"))
    float CurrentCycleTime = 0.0f;

    /** Whether a transition is currently blending */
    UPROPERTY(BlueprintReadOnly, Category = "Lighting|State",
              meta = (AllowPrivateAccess = "true"))
    bool bIsTransitioning = false;

    // ── Blueprint-callable API ────────────────────────────────────────────────

    /** Immediately apply a time-of-day palette (no blend) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    /** Begin a blended transition to a new time-of-day state */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToTimeOfDay(ELight_TimeOfDay TargetState, float BlendDuration);

    /** Get the current palette data for a given time-of-day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    FLight_SunPalette GetPaletteForState(ELight_TimeOfDay State) const;

    /** Force-refresh all scene lighting from current palette */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void RefreshSceneLighting();

    /** Register the default 6-state palette data */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitialiseDefaultPalettes();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    /** Apply palette data to the directional light */
    void ApplySunPalette(const FLight_SunPalette& Palette);

    /** Apply palette data to the fog component */
    void ApplyFogPalette(const FLight_SunPalette& Palette);

    /** Apply palette data to the sky light */
    void ApplySkyLightPalette(const FLight_SunPalette& Palette);

    /** Target state for ongoing transition */
    ELight_TimeOfDay TransitionTarget = ELight_TimeOfDay::Midday;

    /** Elapsed time in current transition */
    float TransitionElapsed = 0.0f;

    /** Duration of current transition */
    float ActiveTransitionDuration = 8.0f;

    /** Palette snapshot at transition start */
    FLight_SunPalette TransitionStartPalette;
};
