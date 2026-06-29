#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightingAtmosphereSystem.generated.h"

/**
 * ELight_TimeOfDay — Prehistoric world time-of-day palette states.
 * Each state drives a distinct cinematic lighting mood.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    PreDawn        UMETA(DisplayName = "Pre-Dawn"),
    Dawn           UMETA(DisplayName = "Dawn"),
    GoldenHour     UMETA(DisplayName = "Golden Hour"),
    Midday         UMETA(DisplayName = "Midday"),
    Afternoon      UMETA(DisplayName = "Afternoon"),
    Dusk           UMETA(DisplayName = "Dusk"),
    BlueHour       UMETA(DisplayName = "Blue Hour"),
    Night          UMETA(DisplayName = "Night"),
    StormOvercast  UMETA(DisplayName = "Storm Overcast")
};

/**
 * FLight_SunParams — Parameters for directional sun light per time-of-day.
 */
USTRUCT(BlueprintType)
struct FLight_SunParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float PitchDegrees = -8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float YawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.72f, 0.35f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float VolumetricScatteringIntensity = 1.8f;
};

/**
 * FLight_FogParams — Exponential height fog parameters per time-of-day.
 */
USTRUCT(BlueprintType)
struct FLight_FogParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor InscatteringColor = FLinearColor(0.95f, 0.65f, 0.35f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float HeightFalloff = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.2f;
};

/**
 * FLight_PaletteEntry — Complete lighting palette for one time-of-day state.
 */
USTRUCT(BlueprintType)
struct FLight_PaletteEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLight_SunParams SunParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLight_FogParams FogParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SkyLightIntensity = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor AmbientColor = FLinearColor(0.45f, 0.38f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float TransitionDurationSeconds = 120.0f;
};

/**
 * ALightingAtmosphereSystem
 *
 * Director-of-photography actor for the prehistoric survival game.
 * Manages cinematic day/night cycle, weather atmosphere, and Lumen-driven
 * volumetric lighting. Each time-of-day state has a curated palette
 * designed to serve the emotional intent of the moment — not just render.
 *
 * Cycle history:
 *   v58 — Dusk palette (sun -5°, amber fog, 3 fill lights)
 *   v59 — Night palette (sun -15°, deep blue fog, 3 night fills)
 *   v60 — Dawn palette (sun -30°, blue-violet fog, god-ray SpotLight)
 *   v61 — Golden Hour palette (sun -8°, amber fog, warm/violet fills, canopy god-ray)
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "TranspersonalGame|Lighting")
class TRANSPERSONALGAME_API ALightingAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Time of Day ──────────────────────────────────────────────────────────

    /** Current active time-of-day state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    /** Whether the day/night cycle advances automatically. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bAutoCycle = true;

    /** Real-time seconds per full in-game day (default: 1200s = 20 min). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay", meta = (ClampMin = "60.0", ClampMax = "7200.0"))
    float RealSecondsPerDay = 1200.0f;

    /** Current in-game time as normalized [0,1] fraction of a full day. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|TimeOfDay")
    float DayFraction = 0.35f;  // Start at golden hour

    // ── Palette Library ───────────────────────────────────────────────────────

    /** All time-of-day lighting palettes. Editable in Blueprint/Details. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    TArray<FLight_PaletteEntry> Palettes;

    // ── Scene References ─────────────────────────────────────────────────────

    /** Reference to the scene's directional sun light. Auto-found on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the scene's exponential height fog. Auto-found on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* SceneFog = nullptr;

    /** Reference to the scene's sky light. Auto-found on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SceneSkyLight = nullptr;

    // ── Lumen Settings ───────────────────────────────────────────────────────

    /** Enable Lumen Global Illumination. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGI = true;

    /** Enable Lumen Reflections. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflections = true;

    /** Enable Volumetric Fog. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bVolumetricFog = true;

    // ── Blueprint Events ─────────────────────────────────────────────────────

    /** Called when the time-of-day state transitions to a new palette. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay, ELight_TimeOfDay PreviousTimeOfDay);

    /** Force an immediate transition to a specific time-of-day palette. */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, bool bInstant = false);

    /** Get the current palette entry for the active time-of-day. */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    FLight_PaletteEntry GetCurrentPalette() const;

    /** Apply a palette entry immediately to all scene lights. */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void ApplyPalette(const FLight_PaletteEntry& Palette);

    /** Manually trigger CAP enforcement (fog dedup, sun pitch guard, FastSkyLUT). */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting|Debug")
    void EnforceCAP();

private:
    void FindSceneActors();
    void AdvanceDayCycle(float DeltaTime);
    ELight_TimeOfDay DayFractionToTimeOfDay(float Fraction) const;
    void BuildDefaultPalettes();

    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::GoldenHour;
    float TransitionElapsed = 0.0f;
    bool bTransitioning = false;
    FLight_PaletteEntry TransitionSource;
    FLight_PaletteEntry TransitionTarget;
};
