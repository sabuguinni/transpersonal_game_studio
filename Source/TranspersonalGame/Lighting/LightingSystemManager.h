#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
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
    DeepNight   UMETA(DisplayName = "Deep Night")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Tense       UMETA(DisplayName = "Tense"),
    Threatening UMETA(DisplayName = "Threatening"),
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Serene      UMETA(DisplayName = "Serene")
};

USTRUCT(BlueprintType)
struct FLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ColorTemperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Exposure = 0.0f;
};

UCLASS()
class TRANSPERSONALGAME_API ALightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clouds")
    class UVolumetricCloudComponent* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
    class UPostProcessComponent* PostProcessComponent;

    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = 0.0, ClampMax = 24.0))
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationInMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoTimeProgression = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeatherState = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherProgression = true;

    // Emotional Tone System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionalIntensity = 0.5f; // 0-1 range

    // Lighting Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ETimeOfDay, FLightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherState, FLightingPreset> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EEmotionalTone, FLightingPreset> EmotionalPresets;

public:
    // Time Control Functions
    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDayDuration(float NewDurationInMinutes);

    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherState(EWeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWeatherState NewWeatherState, float TransitionDuration = 5.0f);

    // Emotional Tone Functions
    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void SetEmotionalTone(EEmotionalTone NewTone, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void BlendToEmotionalTone(EEmotionalTone NewTone, float Intensity, float BlendDuration = 3.0f);

    // Zone-based Lighting
    UFUNCTION(BlueprintCallable, Category = "Zones")
    void ApplyZoneLighting(const FString& ZoneName, float BlendDuration = 2.0f);

    // Threat Response System
    UFUNCTION(BlueprintCallable, Category = "Threat")
    void TriggerThreatLighting(float ThreatLevel = 1.0f, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Threat")
    void RestoreNormalLighting(float RestoreDuration = 5.0f);

private:
    // Internal update functions
    void UpdateTimeProgression(float DeltaTime);
    void UpdateWeatherProgression(float DeltaTime);
    void UpdateLightingComponents();
    void ApplyLightingPreset(const FLightingPreset& Preset, float BlendWeight = 1.0f);
    void BlendLightingPresets(const FLightingPreset& PresetA, const FLightingPreset& PresetB, float BlendFactor);
    
    // Utility functions
    FLightingPreset GetCurrentTimePreset() const;
    FLightingPreset GetCurrentWeatherPreset() const;
    FLightingPreset GetCurrentEmotionalPreset() const;
    float CalculateSunAngle(float TimeOfDay) const;
    
    // Weather transition variables
    bool bIsTransitioningWeather = false;
    EWeatherState TargetWeatherState;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    
    // Emotional tone transition variables
    bool bIsTransitioningEmotion = false;
    EEmotionalTone TargetEmotionalTone;
    float EmotionalTransitionTimer = 0.0f;
    float EmotionalTransitionDuration = 3.0f;
    float TargetEmotionalIntensity = 0.5f;
    
    // Threat system variables
    bool bThreatActive = false;
    float ThreatTimer = 0.0f;
    float ThreatDuration = 10.0f;
    float ThreatLevel = 1.0f;
    FLightingPreset PreThreatPreset;
};