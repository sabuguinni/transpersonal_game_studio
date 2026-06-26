#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "CretaceousLightingSystem.generated.h"

/**
 * Lighting preset for different times of day in the Cretaceous world.
 * Each preset defines sun angle, color temperature, fog density, and atmosphere parameters.
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
    Midnight    UMETA(DisplayName = "Midnight"),
};

/**
 * Weather state affecting atmospheric scattering and fog.
 */
UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
};

/**
 * Lighting parameters for a specific time-of-day preset.
 */
USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    /** Sun pitch angle in degrees (negative = above horizon) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -38.0f;

    /** Sun yaw angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 45.0f;

    /** Sun light intensity (lux) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    /** Sun color (linear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    /** SkyLight intensity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    /** Exponential height fog density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    /** Fog inscattering color (linear) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.85f, 0.72f, 0.55f, 1.0f);

    /** Rayleigh scattering scale for SkyAtmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.0331f;

    /** Mie scattering scale for SkyAtmosphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.004f;

    FLight_TimeOfDayPreset() = default;
};

/**
 * ACretaceousLightingSystem
 *
 * Manages the complete Cretaceous-era lighting environment:
 * - Dynamic day/night cycle with smooth transitions
 * - Lumen GI + hardware ray tracing configuration
 * - SkyAtmosphere with period-accurate atmospheric scattering
 * - Volumetric fog for jungle humidity and morning mist
 * - Weather state transitions (clear → hazy → stormy)
 *
 * Place one instance in the level. It auto-discovers and configures
 * DirectionalLight, SkyLight, SkyAtmosphere, and ExponentialHeightFog actors.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ─── Day/Night Cycle ───────────────────────────────────────────────────

    /** Enable automatic day/night cycle progression */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = false;

    /** Speed multiplier for the day/night cycle (1.0 = real time, 100.0 = 100x faster) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10000.0"))
    float DayCycleSpeed = 100.0f;

    /** Current time of day in hours (0-24) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 15.5f;  // Default: late afternoon golden hour

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Hazy;

    // ─── Lighting References (auto-discovered if null) ─────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    // ─── Presets ───────────────────────────────────────────────────────────

    /** Time-of-day presets defining lighting for each phase */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TArray<FLight_TimeOfDayPreset> TimeOfDayPresets;

    // ─── Blueprint-callable functions ─────────────────────────────────────

    /** Apply a specific time-of-day preset immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    /** Apply dawn lighting preset */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

    /** Apply midday lighting preset */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyMiddayPreset();

    /** Apply night lighting preset */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

    /** Set weather state and transition atmosphere */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeather, float TransitionDuration = 5.0f);

    /** Get current sun pitch angle */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lighting")
    float GetCurrentSunPitch() const { return CurrentSunPitch; }

    /** Get current time of day as enum */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

private:
    void DiscoverLightingActors();
    void ApplyPreset(const FLight_TimeOfDayPreset& Preset);
    void TickDayCycle(float DeltaTime);
    FLight_TimeOfDayPreset InterpolatePresets(const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const;
    FLight_TimeOfDayPreset GetPresetForHour(float Hour) const;
    void InitializeDefaultPresets();

    float CurrentSunPitch = -38.0f;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Hazy;
};
