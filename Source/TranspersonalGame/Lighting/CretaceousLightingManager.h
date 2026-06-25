#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

/**
 * Lighting time-of-day phases for the Cretaceous world.
 * Controls sun angle, color temperature, and atmospheric density.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    StormApproach UMETA(DisplayName = "Storm Approach"),
};

/**
 * Weather state affecting atmosphere and fog density.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rainy       UMETA(DisplayName = "Rainy"),
};

/**
 * Lighting configuration snapshot for a given time/weather combination.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.6f;
};

/**
 * ACretaceousLightingManager
 *
 * Manages the full Cretaceous lighting stack:
 *   - Dynamic day/night cycle with configurable speed
 *   - Weather transitions (clear, overcast, stormy, foggy)
 *   - Lumen GI integration (DirectionalLight + SkyAtmosphere + SkyLight)
 *   - Volumetric fog with Cretaceous atmospheric haze
 *   - PostProcess color grading per time-of-day
 *
 * Place one instance in MinPlayableMap. It auto-discovers lighting actors
 * by class on BeginPlay and drives them via timeline interpolation.
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeOfDayNormalized = 0.35f;

    /** Speed multiplier for day/night cycle (1.0 = real-time, 100.0 = fast cycle) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float DayCycleSpeed = 60.0f;

    /** Whether the day/night cycle is actively ticking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    bool bDayCycleActive = false;

    /** Current resolved time-of-day phase */
    UPROPERTY(BlueprintReadOnly, Category = "Time of Day", meta = (AllowPrivateAccess = "true"))
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Morning;

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    // ── Lighting References (auto-discovered on BeginPlay) ───────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    ADirectionalLight* SunActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    AExponentialHeightFog* FogActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting References")
    APostProcessVolume* PostProcessActor = nullptr;

    // ── Atmosphere Presets ───────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig MorningConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig MiddayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig DuskConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig NightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Presets")
    FLight_AtmosphereConfig StormConfig;

    // ── Blueprint-callable Functions ─────────────────────────────────────────

    /** Immediately apply a lighting config snapshot */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmosphereConfig(const FLight_AtmosphereConfig& Config);

    /** Smoothly transition to a new weather state over TransitionDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(ELight_WeatherState NewWeather, float TransitionDuration = 10.0f);

    /** Force a specific time of day (0.0-1.0) */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    /** Get the current sun pitch angle in degrees */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetCurrentSunPitch() const;

    /** Get the current atmosphere config for the active time/weather */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    FLight_AtmosphereConfig GetCurrentAtmosphereConfig() const;

    /** Discover and cache all lighting actors in the current level */
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void DiscoverLightingActors();

private:
    void UpdateDayCycle(float DeltaTime);
    void UpdateSunTransform();
    void UpdateAtmosphereForTime();
    ELight_TimeOfDay ResolveTimePhase(float NormalizedTime) const;
    FLight_AtmosphereConfig InterpolateConfigs(const FLight_AtmosphereConfig& A, const FLight_AtmosphereConfig& B, float Alpha) const;

    // Weather transition state
    bool bTransitioningWeather = false;
    float WeatherTransitionAlpha = 0.0f;
    float WeatherTransitionDuration = 10.0f;
    ELight_WeatherState TargetWeather = ELight_WeatherState::Clear;
    FLight_AtmosphereConfig TransitionFromConfig;
    FLight_AtmosphereConfig TransitionToConfig;

    void InitDefaultPresets();
};
