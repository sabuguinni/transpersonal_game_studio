#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightAtmosphereManager.generated.h"

/**
 * Lighting palette presets for different times of day.
 * Used by ALightAtmosphereManager to drive Lumen-based dynamic lighting.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

/**
 * Weather state affecting atmospheric scattering and fog density.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

/**
 * Lighting palette data for a specific time-of-day + weather combination.
 */
USTRUCT(BlueprintType)
struct FLight_AtmospherePalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.75f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor SkyFillColor = FLinearColor(0.4f, 0.65f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor GroundBounceColor = FLinearColor(0.9f, 0.55f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    FLinearColor FogInscatteringColor = FLinearColor(0.55f, 0.72f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float FogHeightFalloff = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palette")
    float VolumetricFogExtinctionScale = 0.8f;
};

/**
 * ALightAtmosphereManager
 *
 * Manages the complete lighting and atmospheric state of the Cretaceous world.
 * Controls day/night cycle, weather transitions, Lumen settings, and fill light palettes.
 * Designed for the prehistoric dinosaur survival game — no spiritual/mystical content.
 *
 * Placement: Place one instance in MinPlayableMap. Assign DirectionalLight, SkyLight,
 * and ExponentialHeightFog references in the Details panel.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Light Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Time of Day ───────────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight again) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float TimeOfDayNormalized = 0.5f;

    /** Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min per hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float DayNightCycleSpeed = 60.0f;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    bool bDayNightCycleActive = false;

    /** Current discrete time of day enum (derived from TimeOfDayNormalized) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting|Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    // ─── Weather ────────────────────────────────────────────────────────────────

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Weather transition duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionDuration = 30.0f;

    // ─── Scene References ───────────────────────────────────────────────────────

    /** Reference to the main directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    /** Reference to the exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFogActor = nullptr;

    // ─── Palettes ───────────────────────────────────────────────────────────────

    /** Palette for midday clear sky — harsh overhead sun, cool blue fill, warm ground bounce */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette MiddayPalette;

    /** Palette for golden hour — warm amber sun, deep orange fill, purple shadow */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette GoldenHourPalette;

    /** Palette for dawn — cool blue-pink, low sun angle, heavy ground fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette DawnPalette;

    /** Palette for night — moonlight blue-silver, deep shadow, bioluminescent hints */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Palettes")
    FLight_AtmospherePalette NightPalette;

    // ─── Blueprint Events ───────────────────────────────────────────────────────

    /** Called when time of day transitions to a new discrete state */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    /** Called when weather state changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting|Events")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ─── Blueprint Callable ─────────────────────────────────────────────────────

    /** Apply a specific atmosphere palette immediately (no transition) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyPalette(const FLight_AtmospherePalette& Palette);

    /** Smoothly transition to a target palette over Duration seconds */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToPalette(const FLight_AtmospherePalette& TargetPalette, float Duration);

    /** Force a specific time of day (snaps immediately) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTime);

    /** Force a weather state change (triggers transition) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(ELight_WeatherState NewWeather);

    /** Get the current normalized time (0.0-1.0) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetTimeOfDayNormalized() const { return TimeOfDayNormalized; }

    /** Returns true if it is currently daytime (0.25 to 0.75) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool IsDaytime() const;

    /** Returns true if it is currently nighttime */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool IsNighttime() const;

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateTimeOfDayEnum();
    void ApplyPaletteToScene(const FLight_AtmospherePalette& Palette);

    // Transition state
    bool bTransitioning = false;
    float TransitionElapsed = 0.0f;
    float TransitionDuration = 0.0f;
    FLight_AtmospherePalette TransitionSourcePalette;
    FLight_AtmospherePalette TransitionTargetPalette;

    ELight_TimeOfDay PreviousTimeOfDay = ELight_TimeOfDay::Midday;
    ELight_WeatherState PreviousWeather = ELight_WeatherState::Clear;
};
