#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

/**
 * Lighting preset for different times of day / weather conditions.
 * All types prefixed with Light_ per project naming convention.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Fog         UMETA(DisplayName = "Fog"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FLight_DayPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.88f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.018f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogMaxOpacity = 0.88f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float IndirectLightingIntensity = 1.8f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainfallIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float ThunderProbability = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDurationSeconds = 30.0f;
};

/**
 * ACretaceousLightingManager
 *
 * Manages the full dynamic lighting stack for the Cretaceous prehistoric world.
 * Controls day/night cycle, weather transitions, Lumen GI, volumetric fog,
 * and post-process grading. Designed as a singleton actor placed in MinPlayableMap.
 *
 * Architecture: Lighting & Atmosphere Agent (#08)
 * Dependency: Placed in world by level designer / UE5 Python setup script.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Time of Day ──────────────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeOfDayNormalized = 0.35f; // Default: morning

    /** Speed multiplier for day/night cycle (1.0 = real-time, 60.0 = 1 min/day) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayCycleSpeed = 60.0f;

    /** Whether the day/night cycle is actively running */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = true;

    /** Current discrete time-of-day phase (computed from TimeOfDayNormalized) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::GoldenHour;

    // ── Weather ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLight_WeatherSettings TargetWeather;

    // ── Light References ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    AExponentialHeightFog* FogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    APostProcessVolume* PostProcessVolume = nullptr;

    // ── Preset Settings ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPhaseSettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPhaseSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPhaseSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_DayPhaseSettings NightSettings;

    // ── Blueprint-callable API ───────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDayPreset(ELight_TimeOfDay Phase);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(ELight_WeatherState NewWeather, float TransitionSeconds);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunPitchForTime(float NormalizedTime) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetSkyColorForTime(float NormalizedTime) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void AutoDiscoverLightActors();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyGoldenHourNow();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyMiddayNow();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyNightNow();

private:
    void UpdateDayCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyPhaseSettings(const FLight_DayPhaseSettings& Settings);
    void InitializeDefaultPresets();
    ELight_TimeOfDay ComputeTimePhase(float NormalizedTime) const;

    float WeatherTransitionElapsed = 0.0f;
    float WeatherTransitionDuration = 30.0f;
    bool bWeatherTransitioning = false;
};
