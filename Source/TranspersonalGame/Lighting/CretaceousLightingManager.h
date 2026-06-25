#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "CretaceousLightingManager.generated.h"

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

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    HeatHaze    UMETA(DisplayName = "Heat Haze"),
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Pitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Yaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float Intensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    bool bAtmosphereSunLight = true;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.45f, 0.6f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogExtinctionScale = 0.5f;
};

/**
 * ACretaceousLightingManager
 * Manages the complete Cretaceous-era lighting system including:
 * - Dynamic day/night cycle
 * - Weather state transitions
 * - Lumen GI configuration
 * - Atmospheric scattering (SkyAtmosphere)
 * - Volumetric fog
 * - Post-process color grading
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === DAY/NIGHT CYCLE ===

    /** Current time of day in hours (0.0 - 24.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 10.0f;

    /** Speed of day/night cycle (1.0 = real time, 60.0 = 1 min = 1 hour) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayCycleSpeed = 30.0f;

    /** Whether the day/night cycle is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bDayCycleActive = true;

    /** Current time of day enum */
    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDayEnum = ELight_TimeOfDay::Morning;

    // === WEATHER ===

    /** Current weather state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    /** Weather transition speed (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.1f;

    // === LIGHTING REFERENCES ===

    /** Reference to the directional light (sun) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ADirectionalLight> SunLight;

    /** Reference to the sky light */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<ASkyLight> SkyLightActor;

    /** Reference to the exponential height fog */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    /** Reference to the post process volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    TObjectPtr<APostProcessVolume> PostProcessVolumeActor;

    // === SUN CONFIGURATION ===

    /** Sun configuration for golden hour (default Cretaceous look) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLight_SunConfig GoldenHourSunConfig;

    /** Sun configuration for midday */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Config")
    FLight_SunConfig MiddaySunConfig;

    // === FOG CONFIGURATION ===

    /** Fog configuration for clear weather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Config")
    FLight_FogConfig ClearWeatherFog;

    /** Fog configuration for stormy weather */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Config")
    FLight_FogConfig StormyWeatherFog;

    // === FUNCTIONS ===

    /** Set the time of day directly (0.0 - 24.0) */
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTime);

    /** Set weather state with transition */
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeather);

    /** Apply golden hour preset immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyGoldenHourPreset();

    /** Apply midday preset immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyMiddayPreset();

    /** Apply night preset immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyNightPreset();

    /** Apply storm preset immediately */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Presets")
    void ApplyStormPreset();

    /** Auto-find lighting actors in the level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void AutoFindLightingActors();

    /** Get current time of day as formatted string (HH:MM) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    FString GetTimeOfDayString() const;

    /** Get sun pitch angle for given time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    float GetSunPitchForTime(float TimeOfDay) const;

    /** Get sun color for given time of day */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Day/Night Cycle")
    FLinearColor GetSunColorForTime(float TimeOfDay) const;

protected:
    /** Update sun position based on current time */
    void UpdateSunPosition(float DeltaTime);

    /** Update fog based on weather */
    void UpdateFogForWeather();

    /** Update time of day enum */
    void UpdateTimeOfDayEnum();

    /** Elapsed time accumulator */
    float TimeAccumulator = 0.0f;
};
