#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightingAtmosphereManager.generated.h"

/**
 * Lighting preset for different times of day and weather conditions.
 * Cretaceous prehistoric world — realistic atmospheric lighting.
 */
UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain")
};

USTRUCT(BlueprintType)
struct FLight_DayPhaseSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AmbientOcclusionIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FLight_AtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFogEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogScatteringDistribution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bLumenGIEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bLumenReflectionsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bSkyAtmosphereEnabled = true;
};

/**
 * ALightingAtmosphereManager
 * 
 * Manages the dynamic day/night cycle, weather states, and atmospheric
 * lighting for the Cretaceous prehistoric world.
 * 
 * Integrates with UE5 Lumen for global illumination and reflections.
 * Handles DirectionalLight, ExponentialHeightFog, SkyLight, and SkyAtmosphere.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Day/Night Cycle ───────────────────────────────────────────────────

    /** Current time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeOfDayNormalized = 0.35f;

    /** Speed multiplier for the day/night cycle (1.0 = real-time, 60.0 = 1 min/day) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayCycleSpeed = 60.0f;

    /** Whether the day/night cycle is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = true;

    /** Current time of day enum (derived from TimeOfDayNormalized) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Morning;

    // ─── Weather ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ─── Lighting References ───────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* HeightFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyLight* SkyLightActor = nullptr;

    // ─── Presets ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_DayPhaseSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_DayPhaseSettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_DayPhaseSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_DayPhaseSettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLight_AtmosphereConfig AtmosphereConfig;

    // ─── Blueprint-callable functions ──────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_DayPhaseSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunriseTime() const { return 0.25f; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunsetTime() const { return 0.75f; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindAndCacheLightingActors();

    UFUNCTION(CallInEditor, Category = "Lighting")
    void ApplyCretaceousLightingNow();

private:
    void UpdateDayCycle(float DeltaTime);
    void UpdateSunPosition();
    void UpdateFogForWeather();
    FLight_DayPhaseSettings InterpolateSettings(
        const FLight_DayPhaseSettings& A,
        const FLight_DayPhaseSettings& B,
        float Alpha) const;

    float CachedTimeAccumulator = 0.0f;
};
