#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
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
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rainy       UMETA(DisplayName = "Rainy")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    // Sun/Moon parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    // Sky parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    // Atmospheric parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericFogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float VolumetricFogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    // Post-processing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Exposure = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 1.0f;
};

/**
 * Core lighting system manager that orchestrates all lighting, atmosphere, and time-of-day systems.
 * Implements cinematic lighting principles with performance optimization for open-world gameplay.
 */
UCLASS(ClassGroup=(Lighting), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULightingSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f; // Real-time multiplier

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeHours = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime = true;

    // Weather system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeatherState = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherChanges = true;

    // Lighting configurations for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<ETimeOfDay, FLightingConfiguration> TimeOfDayConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    TMap<EWeatherState, FLightingConfiguration> WeatherConfigurations;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLumenGlobalIllumination = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LightingUpdateFrequency = 0.1f; // Seconds between updates

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetWeatherState(EWeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    ETimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void TransitionToLightingConfiguration(const FLightingConfiguration& TargetConfig, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void ApplyLightingConfiguration(const FLightingConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void InitializeLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting System")
    void SetupVolumetricFog();

private:
    // Internal lighting actors
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ADirectionalLight* MoonLight;

    UPROPERTY()
    class ASkyLight* SkyLight;

    UPROPERTY()
    APostProcessVolume* MainPostProcessVolume;

    // Transition state
    FLightingConfiguration CurrentConfig;
    FLightingConfiguration TargetConfig;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
    float LastUpdateTime = 0.0f;

    // Internal functions
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeather(float DeltaTime);
    void UpdateLighting(float DeltaTime);
    void InterpolateLightingConfiguration(float Alpha);
    
    ETimeOfDay CalculateTimeOfDay() const;
    FLightingConfiguration GetBlendedConfiguration() const;
    
    void FindOrCreateLightingActors();
    void SetupDefaultConfigurations();
    
    // Cinematic lighting helpers
    void ApplyCinematicPrinciples();
    void UpdateSunPosition();
    void UpdateAtmosphericEffects();
    void OptimizePerformance();
};