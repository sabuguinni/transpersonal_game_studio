#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
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
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Humid       UMETA(DisplayName = "Humid")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunPitchDegrees = -35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunYawDegrees = -60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VolumetricFogExtinction = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float BloomIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float VignetteIntensity = 0.35f;
};

USTRUCT(BlueprintType)
struct FLight_DayNightCycleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayLengthSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float NightLengthSeconds = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float TransitionDurationSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    bool bEnableDynamicCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float StartTimeNormalized = 0.3f;
};

/**
 * ACretaceousLightingManager
 * Manages the complete lighting environment for the Cretaceous prehistoric world.
 * Controls day/night cycle, weather states, Lumen GI, volumetric fog, and atmospheric scattering.
 * Designed for photorealistic prehistoric survival game aesthetics.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting Manager"))
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // === DAY/NIGHT CYCLE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    FLight_DayNightCycleConfig DayNightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentTimeNormalized = 0.3f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    float GetCurrentTimeNormalized() const { return CurrentTimeNormalized; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void ApplyMidnightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Day/Night Cycle")
    void ApplyStormPreset();

    // === WEATHER ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Humid;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    ELight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    // === LIGHT REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    AActor* FogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    AActor* PostProcessActor = nullptr;

    // === PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings GoldenHourSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings MidnightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    FLight_TimeOfDaySettings StormSettings;

    // === LUMEN SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableVirtualShadowMaps = true;

    UFUNCTION(BlueprintCallable, Category = "Lumen")
    void ApplyLumenSettings();

    // === UTILITY ===

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void AutoFindLightActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
    void ApplyCurrentSettings();

private:
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings);
    ELight_TimeOfDay NormalizedTimeToEnum(float NormalizedTime) const;
    FLight_TimeOfDaySettings InterpolateSettings(const FLight_TimeOfDaySettings& A, const FLight_TimeOfDaySettings& B, float Alpha) const;
    void InitializeDefaultPresets();

    float AccumulatedTime = 0.0f;
};
