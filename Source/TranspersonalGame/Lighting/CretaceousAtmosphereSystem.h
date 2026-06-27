#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "CretaceousAtmosphereSystem.generated.h"

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
    MidNight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Raining     UMETA(DisplayName = "Raining"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity;

    FLight_TimeOfDayPreset()
        : TimeOfDay(ELight_TimeOfDay::Midday)
        , SunPitchDegrees(-60.0f)
        , SunYawDegrees(45.0f)
        , SunColor(1.0f, 0.95f, 0.85f, 1.0f)
        , SunIntensity(10.0f)
        , FogColor(0.6f, 0.75f, 0.9f, 1.0f)
        , FogDensity(0.02f)
        , SkyLightIntensity(1.0f)
    {}
};

USTRUCT(BlueprintType)
struct FLight_WeatherPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColorTint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableRain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableLightning;

    FLight_WeatherPreset()
        : WeatherState(ELight_WeatherState::Clear)
        , FogDensityMultiplier(1.0f)
        , SunIntensityMultiplier(1.0f)
        , FogColorTint(1.0f, 1.0f, 1.0f, 1.0f)
        , bEnableRain(false)
        , bEnableLightning(false)
    {}
};

/**
 * ACretaceousAtmosphereSystem
 * Manages the complete Cretaceous-era day/night cycle, weather, and atmospheric lighting.
 * Drives DirectionalLight (sun), ExponentialHeightFog (volumetric), and SkyLight.
 * Designed for Unreal Engine 5 Lumen global illumination.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Atmosphere System"))
class TRANSPERSONALGAME_API ACretaceousAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Day/Night Cycle ─────────────────────────────────────────────────────

    /** Current time of day in hours (0.0 = midnight, 12.0 = noon, 24.0 = next midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay;

    /** Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min per game hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float DayCycleSpeed;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive;

    // ── Weather ─────────────────────────────────────────────────────────────

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather;

    /** Duration in seconds before weather can change again */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "30.0", ClampMax = "3600.0"))
    float WeatherChangeCooldown;

    /** Probability (0-1) of weather changing each cycle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherChangeChance;

    // ── Lighting References ──────────────────────────────────────────────────

    /** Reference to the scene's directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the scene's exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AExponentialHeightFog> SceneFog;

    /** Reference to the scene's sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ASkyLight> SceneSkyLight;

    // ── Presets ──────────────────────────────────────────────────────────────

    /** Time of day lighting presets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TArray<FLight_TimeOfDayPreset> TimeOfDayPresets;

    /** Weather presets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TArray<FLight_WeatherPreset> WeatherPresets;

    // ── Lumen Settings ───────────────────────────────────────────────────────

    /** Enable Lumen Global Illumination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bLumenGIEnabled;

    /** Enable Lumen Reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bLumenReflectionsEnabled;

    /** Enable Volumetric Fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bVolumetricFogEnabled;

    // ── Blueprint Events ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmosphere")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Atmosphere")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ── Blueprint Callable Functions ─────────────────────────────────────────

    /** Set time of day immediately (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Transition to a new weather state over the given duration */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherState NewWeather, float TransitionDuration);

    /** Apply the current time-of-day preset to scene lights */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyCurrentTimePreset();

    /** Auto-discover scene lighting actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void AutoDiscoverLightingActors();

    /** Get the current time of day enum from float time */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    /** Get sun pitch angle for current time */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    float GetSunPitchForTime(float TimeHours) const;

    /** Get sun color for current time */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    FLinearColor GetSunColorForTime(float TimeHours) const;

private:
    /** Initialize default presets for all times of day */
    void InitializeDefaultPresets();

    /** Update sun position and color based on current time */
    void UpdateSunTransform(float DeltaTime);

    /** Update fog based on time and weather */
    void UpdateFogSettings();

    /** Update sky light intensity */
    void UpdateSkyLight();

    /** Tick for weather change logic */
    void UpdateWeather(float DeltaTime);

    /** Interpolate between two presets */
    FLight_TimeOfDayPreset InterpolatePresets(const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const;

    float WeatherCooldownTimer;
    ELight_TimeOfDay LastTimeOfDay;
    ELight_WeatherState TargetWeather;
    float WeatherTransitionTimer;
    float WeatherTransitionDuration;
    bool bInWeatherTransition;
};
