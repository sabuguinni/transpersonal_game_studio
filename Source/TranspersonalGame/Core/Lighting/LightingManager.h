#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "LightingManager.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
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
    Mysterious  UMETA(DisplayName = "Mysterious"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Hopeful     UMETA(DisplayName = "Hopeful"),
    Melancholic UMETA(DisplayName = "Melancholic"),
    Dramatic    UMETA(DisplayName = "Dramatic")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-30.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Exposure = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FLinearColor ColorGrading = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FTimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTime = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeSpeed = 1.0f; // Multiplier for time progression

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoProgress = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TMap<ETimeOfDay, FLightingConfiguration> TimeConfigurations;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, ETimeOfDay, OldTime, ETimeOfDay, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherState, OldWeather, EWeatherState, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLightingUpdated, const FLightingConfiguration&, NewConfiguration);

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ALightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clouds")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
    class APostProcessVolume* PostProcessVolume;

    // Time and weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FTimeOfDaySettings TimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherState CurrentWeather = EWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Tone")
    EEmotionalTone CurrentEmotionalTone = EEmotionalTone::Peaceful;

    // Lighting configurations for different scenarios
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<EWeatherState, FLightingConfiguration> WeatherConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
    TMap<EEmotionalTone, FLightingConfiguration> EmotionalConfigurations;

    // Current active configuration
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FLightingConfiguration CurrentConfiguration;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    bool bSmoothTransitions = true;

    // Internal state
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    FLightingConfiguration SourceConfiguration;
    FLightingConfiguration TargetConfiguration;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnLightingUpdated OnLightingUpdated;

    // Public API
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherState NewWeather, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetEmotionalTone(EEmotionalTone NewTone, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCurrentTime(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyLightingConfiguration(const FLightingConfiguration& Configuration, bool bImmediate = false);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ETimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetCurrentTime() const { return TimeSettings.CurrentTime; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    EWeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    EEmotionalTone GetCurrentEmotionalTone() const { return CurrentEmotionalTone; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ForceUpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SaveCurrentConfigurationAsPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    bool LoadPreset(const FString& PresetName);

protected:
    // Internal methods
    void InitializeLightingComponents();
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateLightingTransition(float DeltaTime);
    void ApplyConfigurationToComponents(const FLightingConfiguration& Configuration);
    FLightingConfiguration BlendConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha);
    ETimeOfDay TimeToTimeOfDay(float Time) const;
    void SetupDefaultConfigurations();

    // Component update methods
    void UpdateSunLight(const FLightingConfiguration& Configuration);
    void UpdateSkyLight(const FLightingConfiguration& Configuration);
    void UpdateSkyAtmosphere(const FLightingConfiguration& Configuration);
    void UpdateVolumetricClouds(const FLightingConfiguration& Configuration);
    void UpdateHeightFog(const FLightingConfiguration& Configuration);
    void UpdatePostProcess(const FLightingConfiguration& Configuration);

    // Utility methods
    FLinearColor BlendColors(const FLinearColor& A, const FLinearColor& B, float Alpha);
    float BlendFloats(float A, float B, float Alpha);
    FRotator BlendRotators(const FRotator& A, const FRotator& B, float Alpha);
};