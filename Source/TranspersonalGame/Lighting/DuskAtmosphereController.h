#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "DuskAtmosphereController.generated.h"

/**
 * Lighting palette identifiers for the prehistoric world's day/night cycle.
 * Each palette represents a distinct time-of-day with unique emotional tone.
 */
UENUM(BlueprintType)
enum class ELight_DayPalette : uint8
{
    Dawn        UMETA(DisplayName = "Dawn — Golden Sunrise"),
    Midday      UMETA(DisplayName = "Midday — Harsh Overhead Sun"),
    Dusk        UMETA(DisplayName = "Dusk — Golden Hour Sunset"),
    Night       UMETA(DisplayName = "Night — Moonlit Darkness"),
    Overcast    UMETA(DisplayName = "Overcast — Storm Approaching"),
    Volcanic    UMETA(DisplayName = "Volcanic — Ash & Ember Sky")
};

/**
 * Data container for a single lighting palette configuration.
 * All values are physically-based and tuned for Lumen GI.
 */
USTRUCT(BlueprintType)
struct FLight_PaletteConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sky")
    FLinearColor SkyLightColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float MieAnisotropy = 0.72f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float RayleighScale = 0.0331f;
};

/**
 * ADuskAtmosphereController
 *
 * Runtime lighting controller for the prehistoric survival game.
 * Manages day/night cycle transitions, palette blending, and Lumen GI settings.
 * Designed as a singleton actor placed in the persistent level.
 *
 * Director of Photography philosophy: light serves emotional intent, not just rendering.
 * Each palette is crafted to evoke a specific survival emotion:
 *   - Dawn: hope, new beginning, cautious optimism
 *   - Midday: exposure, vulnerability, harsh reality
 *   - Dusk: urgency, beauty before darkness, last chance
 *   - Night: fear, stealth, primal danger
 *   - Overcast: dread, storm survival, oppressive weight
 *   - Volcanic: apocalyptic, awe, existential threat
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dusk Atmosphere Controller"))
class TRANSPERSONALGAME_API ADuskAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ADuskAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // =========================================================
    // PALETTE CONFIGURATION
    // =========================================================

    /** Current active lighting palette */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_DayPalette CurrentPalette = ELight_DayPalette::Dusk;

    /** Target palette for blending transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    ELight_DayPalette TargetPalette = ELight_DayPalette::Night;

    /** Blend alpha [0..1] between CurrentPalette and TargetPalette */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendAlpha = 0.0f;

    /** Speed of automatic palette transition (alpha units per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float TransitionSpeed = 0.05f;

    /** Whether the day/night cycle advances automatically */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    bool bAutoCycle = false;

    /** Duration of a full day/night cycle in real seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Cycle")
    float FullDayDurationSeconds = 1200.0f;

    // =========================================================
    // SCENE REFERENCES (auto-found on BeginPlay if null)
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // =========================================================
    // PALETTE PRESETS
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig DawnPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig MiddayPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig DuskPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig NightPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig OvercastPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_PaletteConfig VolcanicPreset;

    // =========================================================
    // BLUEPRINT-CALLABLE FUNCTIONS
    // =========================================================

    /** Instantly apply a palette with no transition */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void ApplyPaletteImmediate(ELight_DayPalette Palette);

    /** Begin a smooth transition to the target palette */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Control")
    void BeginTransitionTo(ELight_DayPalette NewTarget, float Speed = 0.05f);

    /** Force-find all scene lighting actors (call after level load) */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Setup")
    void AutoFindLightingActors();

    /** Get the palette config for a given palette enum */
    UFUNCTION(BlueprintCallable, Category = "Lighting|Query")
    FLight_PaletteConfig GetPaletteConfig(ELight_DayPalette Palette) const;

    /** Returns current time-of-day as normalized [0..1] (0=midnight, 0.5=noon) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting|Query")
    float GetTimeOfDayNormalized() const;

private:
    void ApplyConfig(const FLight_PaletteConfig& Config);
    FLight_PaletteConfig BlendConfigs(const FLight_PaletteConfig& A, const FLight_PaletteConfig& B, float Alpha) const;
    void InitDefaultPresets();

    float CycleElapsedSeconds = 0.0f;
    bool bTransitioning = false;
};
