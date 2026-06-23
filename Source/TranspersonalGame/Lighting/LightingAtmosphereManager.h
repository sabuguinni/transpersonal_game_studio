#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "LightingAtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    GoldenHour  UMETA(DisplayName = "Golden Hour"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunPitch = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor SunColor = FLinearColor(1.0f, 0.88f, 0.72f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    FLinearColor FogColor = FLinearColor(0.45f, 0.62f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Time")
    float ExposureBias = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float LightIntensityMultiplier = 1.0f;
};

/**
 * ALightingAtmosphereManager
 * Manages the full Cretaceous lighting stack:
 * - Dynamic day/night cycle with configurable time-of-day presets
 * - Weather system (clear, cloudy, rain, storm, fog)
 * - Lumen GI integration (DirectionalLight, SkyLight, SkyAtmosphere)
 * - Volumetric fog and ExponentialHeightFog
 * - PostProcess color grading for cinematic Cretaceous atmosphere
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Lighting & Atmosphere Manager"))
class TRANSPERSONALGAME_API ALightingAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingAtmosphereManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Day/Night Cycle ───────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    ELight_TimeOfDay CurrentTimePreset = ELight_TimeOfDay::GoldenHour;

    // ─── Weather ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherConfig CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    // ─── Actor References ──────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> DirectionalLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AActor> PostProcessActor;

    // ─── Time-of-Day Presets ───────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig DawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig MiddayConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig GoldenHourConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig NightConfig;

    // ─── Blueprint Functions ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void SetTimeOfDay(float NewTimeHours);

    UFUNCTION(BlueprintCallable, Category = "Lighting|DayNight")
    void ApplyTimePreset(ELight_TimeOfDay Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Lumen")
    void ApplyLumenSettings(bool bHighQuality);

    UFUNCTION(BlueprintPure, Category = "Lighting|DayNight")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting|DayNight")
    bool IsNight() const { return CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f; }

    UFUNCTION(BlueprintPure, Category = "Lighting|Weather")
    bool IsRaining() const { return CurrentWeather.RainIntensity > 0.1f; }

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyGoldenHourPreset();

    UFUNCTION(CallInEditor, Category = "Lighting|Debug")
    void ApplyCretaceousAtmosphere();

private:
    float TimeAccumulator = 0.0f;
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;
    float WeatherTransitionProgress = 1.0f;

    void UpdateSunPosition(float TimeHours);
    void UpdateFogForWeather();
    void UpdateSkyLightForWeather();
    FLight_TimeOfDayConfig InterpolateConfigs(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha) const;
    FLight_TimeOfDayConfig GetConfigForTime(float TimeHours) const;
    void ApplyConfig(const FLight_TimeOfDayConfig& Config);
};
