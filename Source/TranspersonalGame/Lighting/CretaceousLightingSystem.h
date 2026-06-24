#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "CretaceousLightingSystem.generated.h"

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
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Stormy      UMETA(DisplayName = "Stormy"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Volcanic    UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Pitch = -42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Yaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.86f, 0.63f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    bool bAtmosphereSunLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float ShadowAmount = 0.85f;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.45f, 0.55f, 0.65f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogMaxOpacity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float StartDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDayPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLight_SunConfig SunConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    FLight_FogConfig FogConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float SkyLightIntensity = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float ExposureBias = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float BloomIntensity = 0.35f;
};

/**
 * ACretaceousLightingSystem
 * Manages the full Cretaceous-era lighting stack:
 * - Dynamic day/night cycle with 8 time-of-day presets
 * - Weather state transitions (clear, stormy, volcanic ash)
 * - Lumen GI integration with real-time sky capture
 * - Volumetric fog and atmospheric perspective
 * - PostProcess color grading for cinematic mood
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Cretaceous Lighting System"))
class TRANSPERSONALGAME_API ACretaceousLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === TIME OF DAY ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float DayNightCycleDurationSeconds = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    bool bEnableDynamicDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|TimeOfDay")
    float CurrentTimeNormalized = 0.6f;

    // === WEATHER ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Weather")
    float WeatherTransitionSpeed = 0.5f;

    // === PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_TimeOfDayPreset> TimeOfDayPresets;

    // === REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    ASkyLight* SkyLightActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    AExponentialHeightFog* HeightFogActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    APostProcessVolume* PostProcessActor = nullptr;

    // === FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting|Weather")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void ApplyNightPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void ApplyStormPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting|TimeOfDay")
    void ApplyVolcanicAshPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyCurrentPreset();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentSunPitch() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentSkyColor() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool IsNightTime() const;

private:
    void InitializeDefaultPresets();
    void UpdateDayNightCycle(float DeltaTime);
    void ApplyPresetToWorld(const FLight_TimeOfDayPreset& Preset);
    FLight_TimeOfDayPreset GetPresetForTime(float NormalizedTime) const;
    FLight_TimeOfDayPreset LerpPresets(const FLight_TimeOfDayPreset& A, const FLight_TimeOfDayPreset& B, float Alpha) const;

    float ElapsedCycleTime = 0.0f;
};
