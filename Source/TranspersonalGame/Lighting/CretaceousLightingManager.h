#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
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
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Hazy        UMETA(DisplayName = "Hazy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rain        UMETA(DisplayName = "Rain"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.878f, 0.62f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.45f, 0.65f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bVolumetricFog = true;
};

USTRUCT(BlueprintType)
struct FLight_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 30.0f;
};

/**
 * ACretaceousLightingManager
 * Manages the full Cretaceous-era lighting stack:
 * - Dynamic day/night cycle
 * - Weather system integration
 * - Lumen GI configuration
 * - Atmospheric scattering
 * - Volumetric fog
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Time of Day ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float CurrentTimeNormalized = 0.55f;

    // --- Weather ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherConfig CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    FLight_WeatherConfig TargetWeather;

    // --- Scene References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AActor* HeightFogActor = nullptr;

    // --- Lumen Settings ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenMaxTraceDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenReflectionsMaxRoughness = 0.8f;

    // --- Golden Hour Preset ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig GoldenHourConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig MidnightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    FLight_TimeOfDayConfig DawnConfig;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeOfDayPreset(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLumenConsoleVars();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeNormalized(float TimeNormalized);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetCurrentTimeNormalized() const { return CurrentTimeNormalized; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ForceGoldenHour();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ForceNight();

private:
    float WeatherTransitionElapsed = 0.0f;
    bool bTransitioningWeather = false;

    void TickDayNightCycle(float DeltaTime);
    void TickWeatherTransition(float DeltaTime);
    void UpdateSunFromTimeNormalized(float T);
    FLight_TimeOfDayConfig GetConfigForTime(float T) const;
    FLight_TimeOfDayConfig LerpConfigs(const FLight_TimeOfDayConfig& A, const FLight_TimeOfDayConfig& B, float Alpha) const;
    void ApplyConfigToScene(const FLight_TimeOfDayConfig& Config);
    ELight_TimeOfDay TimeNormalizedToEnum(float T) const;
};
