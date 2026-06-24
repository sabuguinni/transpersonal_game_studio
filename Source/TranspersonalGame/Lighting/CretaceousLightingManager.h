#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
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
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Fog"),
    Haze        UMETA(DisplayName = "Haze")
};

USTRUCT(BlueprintType)
struct FLight_SunConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Pitch = -38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Yaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float Intensity = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    FLinearColor LightColor = FLinearColor(1.0f, 0.878f, 0.620f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    bool bAtmosphereSunLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float IndirectLightingIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Sun")
    float ShadowAmount = 0.85f;
};

USTRUCT(BlueprintType)
struct FLight_FogConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogDensity = 0.025f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    FLinearColor FogColor = FLinearColor(0.55f, 0.72f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float FogMaxOpacity = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Fog")
    float VolumetricFogExtinctionScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_AtmospherePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    ELight_TimeOfDay TimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    ELight_WeatherState WeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    FLight_SunConfig SunConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    FLight_FogConfig FogConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float SkyLightIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float BloomIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float AmbientOcclusionIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    float ColorSaturation = 1.1f;
};

/**
 * ACretaceousLightingManager
 * 
 * Manages the complete Cretaceous-era lighting environment for the prehistoric survival game.
 * Controls the day/night cycle, weather transitions, Lumen GI settings, and atmospheric effects.
 * 
 * Designed to produce cinematic, photorealistic lighting inspired by Roger Deakins' cinematography
 * and the RDR2 art team's approach to environmental lighting.
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
    // === CORE REFERENCES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<ASkyLight> SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|References")
    TObjectPtr<AExponentialHeightFog> HeightFogActor;

    // === DAY/NIGHT CYCLE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float DayDurationSeconds = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 15.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|DayNight")
    float TimeMultiplier = 1.0f;

    // === CURRENT ATMOSPHERE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    FLight_AtmospherePreset CurrentPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    ELight_TimeOfDay ActiveTimeOfDay = ELight_TimeOfDay::GoldenHour;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Atmosphere")
    ELight_WeatherState ActiveWeather = ELight_WeatherState::Clear;

    // === LUMEN SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenGIEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bLumenReflectionsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    float LumenMaxTraceDistance = 32768.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Lumen")
    bool bVolumetricFogEnabled = true;

    // === PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting|Presets")
    TArray<FLight_AtmospherePreset> AtmospherePresets;

    // === FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyAtmospherePreset(const FLight_AtmospherePreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLumenConsoleVars();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureSunLight(const FLight_SunConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureFog(const FLight_FogConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCurrentHour(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentHour() const { return CurrentHour; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ELight_TimeOfDay GetTimeOfDayFromHour(float Hour) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeDefaultPresets();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyGoldenHourPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyMidnightPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyDawnPreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Lighting")
    void ApplyStormPreset();

private:
    void TickDayNightCycle(float DeltaTime);
    void UpdateSunPositionFromHour(float Hour);
    FLinearColor InterpolateSunColor(float Hour) const;
    float GetSunPitchFromHour(float Hour) const;
    float GetSunIntensityFromHour(float Hour) const;

    float AccumulatedTime = 0.0f;
};
