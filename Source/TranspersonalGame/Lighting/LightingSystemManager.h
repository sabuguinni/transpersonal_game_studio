#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Exposure = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Temperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Lighting Actors (will be spawned and managed)
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    APostProcessVolume* PostProcessVolume;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float TimeSpeed = 1.0f; // Multiplier for time passage

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState TargetWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentWeatherBlend = 0.0f;

    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;

    // Lumen Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenGlobalIlluminationQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenReflectionQuality = 1.0f;

    // Volumetric Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricFogDistance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetrics")
    float VolumetricFogStaticLightingScatteringIntensity = 1.0f;

public:
    // Blueprint Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherState NewWeather, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void TransitionToWeather(EWeatherState NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingPreset(const FLightingPreset& Preset, float BlendWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateVolumetricSettings();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnTimeOfDayChanged(ETimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnWeatherChanged(EWeatherState NewWeather);

protected:
    // Internal Functions
    void InitializeLightingActors();
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateLighting();
    void ApplyTimeBasedLighting();
    void ApplyWeatherBasedLighting();
    FLightingPreset BlendLightingPresets(const FLightingPreset& PresetA, const FLightingPreset& PresetB, float Alpha);
    void InitializeDefaultPresets();

private:
    ETimeOfDay LastTimeOfDay;
    float WeatherTransitionTimer;
    float WeatherTransitionDuration;
    bool bWeatherTransitioning;
};