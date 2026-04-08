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
    PartlyCloudy UMETA(DisplayName = "Partly Cloudy"),
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
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Hopeful     UMETA(DisplayName = "Hopeful"),
    Threatening UMETA(DisplayName = "Threatening")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    // Sun/Moon Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moon")
    float MoonIntensity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moon")
    FLinearColor MoonColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    // Sky Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor::White;
    
    // Atmosphere Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScatteringScale = 0.0331f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 0.004f;
    
    // Cloud Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudDensity = 0.7f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudSpeed = 0.1f;
    
    // Fog Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.447f, 0.638f, 1.0f, 1.0f);
    
    // Post Process Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ExposureBias = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FLinearColor ColorGrading = FLinearColor::White;
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
    class UDirectionalLightComponent* MoonLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class USkyAtmosphereComponent* SkyAtmosphere;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clouds")
    class UVolumetricCloudComponent* VolumetricClouds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog")
    class UExponentialHeightFogComponent* HeightFog;

public:
    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeSpeed = 1.0f; // Multiplier for time passage
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoAdvanceTime = true;
    
    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState TargetWeather = EWeatherState::Clear;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 0.5f;
    
    // Emotional Tone System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    EEmotionalTone CurrentTone = EEmotionalTone::Peaceful;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float EmotionalIntensity = 0.5f; // 0-1 scale
    
    // Lighting Configurations for each time/weather/emotion combination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<ETimeOfDay, FLightingConfiguration> TimeBasedConfigs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<EWeatherState, FLightingConfiguration> WeatherBasedConfigs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<EEmotionalTone, FLightingConfiguration> EmotionBasedConfigs;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherState NewWeather, float TransitionTime = 5.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetEmotionalTone(EEmotionalTone NewTone, float Intensity = 0.5f);
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDayEnum() const;
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ForceUpdateLighting();
    
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void CreateDynamicWeatherEvent(EWeatherState Weather, float Duration);

private:
    // Internal update functions
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateLightingConfiguration();
    void ApplyLightingConfiguration(const FLightingConfiguration& Config);
    void BlendLightingConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha, FLightingConfiguration& Result);
    
    // Helper functions
    FLightingConfiguration GetCurrentLightingConfiguration() const;
    float CalculateSunAngle(float TimeHours) const;
    FLinearColor CalculateSunColor(float SunAngle) const;
    FLinearColor CalculateSkyColor(float SunAngle, EWeatherState Weather) const;
    
    // Weather transition
    float WeatherTransitionProgress = 0.0f;
    bool bTransitioningWeather = false;
    
    // Dynamic weather events
    struct FWeatherEvent
    {
        EWeatherState Weather;
        float Duration;
        float RemainingTime;
    };
    
    TArray<FWeatherEvent> ActiveWeatherEvents;
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    float UpdateFrequency = 0.1f; // Update 10 times per second
};