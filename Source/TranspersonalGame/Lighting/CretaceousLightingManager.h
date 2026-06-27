#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
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
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    MidNight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitch = -30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.6f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ExposureMax = 3.0f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float VolcanicAshDensity = 0.0f;
};

/**
 * ACretaceousLightingManager
 * Manages the complete Cretaceous-era lighting system including:
 * - Dynamic day/night cycle with realistic sun arc
 * - Weather state transitions (clear, rain, storm, volcanic ash)
 * - Lumen GI + volumetric fog configuration
 * - PostProcess color grading per time-of-day
 * - Atmospheric scattering for prehistoric jungle mood
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
    // ── Day/Night Cycle ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationSeconds = 600.0f;  // 10 real minutes = 1 game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDayNormalized = 0.35f;  // 0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk

    UPROPERTY(BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    // ── Weather ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableRandomWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeIntervalSeconds = 300.0f;

    // ── Light References ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    AExponentialHeightFog* FogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    APostProcessVolume* PostProcessActor = nullptr;

    // ── Time-of-Day Presets ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings MiddaySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Presets")
    FLight_TimeOfDaySettings NightSettings;

    // ── Lumen Settings ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bUseLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bUseLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenSceneDetail = 1.0f;

    // ── Blueprint-callable Functions ─────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLight_TimeOfDaySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindAndCacheLightActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyNightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyStormPreset();

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetCurrentTimeOfDayNormalized() const { return CurrentTimeOfDayNormalized; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

private:
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateSunPosition(float NormalizedTime);
    void UpdateFogForTimeOfDay(float NormalizedTime);
    void UpdatePostProcessForTimeOfDay(float NormalizedTime);
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void InitializeDefaultPresets();

    float WeatherTransitionTimer = 0.0f;
    float WeatherChangeTimer = 0.0f;
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;
    float CurrentTransitionAlpha = 1.0f;
    bool bIsTransitioningWeather = false;
};
