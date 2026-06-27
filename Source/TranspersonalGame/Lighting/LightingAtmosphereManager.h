#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "LightingAtmosphereManager.generated.h"

// ============================================================
// Lighting & Atmosphere Manager — Agent #08
// Cycle: PROD_CYCLE_AUTO_20260627_010
// Manages dynamic day/night cycle, weather states, and
// Lumen-based atmospheric lighting for the Cretaceous world.
// ============================================================

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

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Haze        UMETA(DisplayName = "Heat Haze")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunPitchAngle = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunYawAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLinearColor SunColor = FLinearColor(1.0f, 0.78f, 0.55f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLinearColor FogColor = FLinearColor(0.6f, 0.75f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SkyLightIntensity = 1.8f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    bool bRainActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float RainIntensity = 0.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Day/Night Cycle ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bDayNightCycleEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayLengthSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float CurrentTimeNormalized = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Afternoon;

    // ---- Scene References ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* AtmosphereFog = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    // ---- Time of Day Configs ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Configs")
    FLight_TimeOfDayConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Configs")
    FLight_TimeOfDayConfig GoldenHourConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Configs")
    FLight_TimeOfDayConfig MiddayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Configs")
    FLight_TimeOfDayConfig NightConfig;

    // ---- Weather ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherConfig CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ---- Lumen Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGIEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflectionsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenSceneDetail = 1.0f;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void SetWeatherState(ELight_WeatherState NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    ELight_WeatherState GetCurrentWeatherState() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting|Lumen")
    void ApplyLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting|Debug")
    void ForceGoldenHour();

    UFUNCTION(BlueprintCallable, Category = "Lighting|Debug")
    void ForceMidnight();

    UFUNCTION(BlueprintCallable, Category = "Lighting|Debug", CallInEditor)
    void ApplyCurrentSettings();

private:
    void UpdateSunPosition(float DeltaTime);
    void UpdateFogForTimeOfDay();
    void UpdateSkyLightForTimeOfDay();
    void InterpolateToConfig(const FLight_TimeOfDayConfig& TargetConfig, float Alpha);
    FLight_TimeOfDayConfig GetConfigForTime(float NormalizedTime) const;
    ELight_TimeOfDay ClassifyTimeOfDay(float NormalizedTime) const;

    float WeatherTransitionAlpha = 0.0f;
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;
    bool bTransitioningWeather = false;
};
