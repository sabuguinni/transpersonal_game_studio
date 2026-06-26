#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "CretaceousLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight"),
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    HeatHaze    UMETA(DisplayName = "Heat Haze"),
    MorningMist UMETA(DisplayName = "Morning Mist"),
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.018f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.6f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float ThunderProbability = 0.0f;
};

/**
 * ACretaceousLightingManager
 * 
 * Manages the full day/night cycle and dynamic weather system for the
 * Cretaceous prehistoric world. Drives Lumen GI, volumetric fog,
 * sky atmosphere, and post-process for cinematic prehistoric atmosphere.
 * 
 * Placed once in MinPlayableMap. Controls all lighting actors via references.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Day/Night Cycle ──────────────────────────────────────────────────────

    /** Current time of day in hours (0-24) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 10.5f;

    /** Speed multiplier for day/night cycle (1.0 = real time, 60.0 = 1min per day) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayCycleSpeed = 120.0f;

    /** Whether the day/night cycle is running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = false;

    /** Current time of day enum */
    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Morning;

    // ── Weather ──────────────────────────────────────────────────────────────

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Target weather state for transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;

    /** Weather transition speed (0-1 blend per second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.05f;

    /** Current weather blend alpha (0=current, 1=target) */
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherBlendAlpha = 0.0f;

    // ── Lighting References ──────────────────────────────────────────────────

    /** Reference to the main directional sun light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight = nullptr;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    // ── Presets ──────────────────────────────────────────────────────────────

    /** Lighting settings for golden hour (morning) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings MorningPreset;

    /** Lighting settings for midday */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings MiddayPreset;

    /** Lighting settings for sunset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings SunsetPreset;

    /** Lighting settings for night */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings NightPreset;

    // ── Blueprint Events ─────────────────────────────────────────────────────

    /** Called when time of day changes to a new period */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting Events")
    void OnTimeOfDayChanged(ELight_TimeOfDay NewTimeOfDay);

    /** Called when weather changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Lighting Events")
    void OnWeatherChanged(ELight_WeatherState NewWeather);

    // ── Public API ───────────────────────────────────────────────────────────

    /** Set time of day directly (0-24 hours) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    /** Trigger a weather transition */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(ELight_WeatherState NewWeather, float TransitionDuration = 30.0f);

    /** Apply a specific lighting preset immediately */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Preset);

    /** Get current sun pitch angle based on time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetSunPitchForTime(float TimeOfDay) const;

    /** Get current sun color based on time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    FLinearColor GetSunColorForTime(float TimeOfDay) const;

    /** Get fog density based on time of day and weather */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetFogDensityForConditions(float TimeOfDay, ELight_WeatherState Weather) const;

    /** Force recapture of sky light */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void RecaptureSkyLight();

    /** Apply Cretaceous default lighting setup */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyCretaceousDefaults();

private:
    void UpdateDayCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateSunPosition(float TimeOfDay);
    void UpdateFogForTimeAndWeather();
    ELight_TimeOfDay GetTimeOfDayEnum(float TimeOfDay) const;
    void InitializePresets();

    float PreviousTimeOfDay = -1.0f;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 30.0f;
};
