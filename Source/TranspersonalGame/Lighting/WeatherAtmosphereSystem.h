#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "WeatherAtmosphereSystem.generated.h"

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear_Sunny         UMETA(DisplayName = "Clear Sunny"),
    Partly_Cloudy       UMETA(DisplayName = "Partly Cloudy"),
    Overcast           UMETA(DisplayName = "Overcast"),
    Light_Rain         UMETA(DisplayName = "Light Rain"),
    Heavy_Rain         UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm       UMETA(DisplayName = "Thunderstorm"),
    Fog_Light          UMETA(DisplayName = "Light Fog"),
    Fog_Dense          UMETA(DisplayName = "Dense Fog"),
    Mist_Morning       UMETA(DisplayName = "Morning Mist"),
    Storm_Approaching  UMETA(DisplayName = "Approaching Storm"),
    Storm_Passing      UMETA(DisplayName = "Passing Storm")
};

UENUM(BlueprintType)
enum class EAtmosphericCondition : uint8
{
    Pristine_Clear      UMETA(DisplayName = "Pristine Clear"),
    Humid_Tropical      UMETA(DisplayName = "Humid Tropical"),
    Dusty_Arid         UMETA(DisplayName = "Dusty Arid"),
    Volcanic_Ash       UMETA(DisplayName = "Volcanic Ash"),
    Pollen_Heavy       UMETA(DisplayName = "Heavy Pollen"),
    Smoke_Wildfire     UMETA(DisplayName = "Wildfire Smoke"),
    Mist_Waterfall     UMETA(DisplayName = "Waterfall Mist"),
    Steam_Geothermal   UMETA(DisplayName = "Geothermal Steam")
};

USTRUCT(BlueprintType)
struct FWeatherConfiguration
{
    GENERATED_BODY()

    // Cloud properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudHeight = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    FLinearColor CloudColor = FLinearColor::White;

    // Precipitation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    float RainDropSize = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
    FVector RainDirection = FVector(0, 0, -1);

    // Wind
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindTurbulence = 0.3f;

    // Fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    // Lighting modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColorTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensityMultiplier = 1.0f;

    // Atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericPerspectiveScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScatteringScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor RayleighScatteringTint = FLinearColor::White;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    // Particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UParticleSystem* WeatherParticles = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ParticleIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FLightningStrike
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float Intensity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float Duration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    FLinearColor Color = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    class USoundCue* ThunderSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float ThunderDelay = 3.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWeatherAtmosphereSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWeatherAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EWeatherType NewWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetAtmosphericCondition(EAtmosphericCondition Condition);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(const FWeatherConfiguration& TargetWeather, float Duration);

    // Cloud control
    UFUNCTION(BlueprintCallable, Category = "Clouds")
    void SetCloudCoverage(float Coverage, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Clouds")
    void SetCloudDensity(float Density, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Clouds")
    void SetCloudMovement(const FVector& Direction, float Speed);

    // Precipitation
    UFUNCTION(BlueprintCallable, Category = "Precipitation")
    void SetRainIntensity(float Intensity, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Precipitation")
    void StartRain(float Intensity = 0.5f, float RampUpTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Precipitation")
    void StopRain(float RampDownTime = 3.0f);

    // Wind system
    UFUNCTION(BlueprintCallable, Category = "Wind")
    void SetWindDirection(const FVector& Direction, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Wind")
    void SetWindTurbulence(float Turbulence);

    UFUNCTION(BlueprintCallable, Category = "Wind")
    FVector GetCurrentWindVector() const;

    // Lightning and thunder
    UFUNCTION(BlueprintCallable, Category = "Lightning")
    void TriggerLightningStrike(const FVector& Location, float Intensity = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Lightning")
    void SetLightningFrequency(float FrequencyPerMinute);

    UFUNCTION(BlueprintCallable, Category = "Lightning")
    void CreateLightningStrike(const FLightningStrike& Strike);

    // Fog control
    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogDensity(float Density, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void SetFogColor(const FLinearColor& Color, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Fog")
    void CreateLocalizedFog(const FVector& Center, float Radius, float Density);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetAtmosphericPerspective(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetScatteringProperties(float MieScale, const FLinearColor& RayleighTint);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLightingForWeather();

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetWeatherQuality(float Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForViewDistance(float Distance);

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnWeatherChanged(EWeatherType NewWeather);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnLightningStrike(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRainStarted(float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnRainStopped();

protected:
    // Current weather state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EWeatherType CurrentWeatherType = EWeatherType::Clear_Sunny;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EAtmosphericCondition CurrentAtmosphericCondition = EAtmosphericCondition::Pristine_Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FWeatherConfiguration CurrentWeatherConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FWeatherConfiguration TargetWeatherConfig;

    // Transition system
    UPROPERTY()
    bool bIsTransitioning = false;

    UPROPERTY()
    float TransitionProgress = 0.0f;

    UPROPERTY()
    float TransitionDuration = 5.0f;

    // Weather presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EWeatherType, FWeatherConfiguration> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EAtmosphericCondition, FWeatherConfiguration> AtmosphericPresets;

    // Component references
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLight;

    UPROPERTY()
    AVolumetricCloud* VolumetricClouds;

    UPROPERTY()
    AExponentialHeightFog* HeightFog;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* ThunderAudioComponent;

    // Particle systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UParticleSystemComponent* RainParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UParticleSystemComponent* LightningParticles;

    // Lightning system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningFrequency = 0.0f; // strikes per minute

    UPROPERTY()
    float TimeSinceLastLightning = 0.0f;

    UPROPERTY()
    TArray<FLightningStrike> ActiveLightningStrikes;

    // Wind system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector BaseWindDirection = FVector(1, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float BaseWindSpeed = 5.0f;

    UPROPERTY()
    float WindVariationTime = 0.0f;

    // Material parameter collection for weather effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* WeatherMPC;

private:
    void InitializeWeatherPresets();
    void InitializeAtmosphericPresets();
    void InitializeComponents();
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherConfiguration(const FWeatherConfiguration& Config);
    void UpdateLightningSystem(float DeltaTime);
    void UpdateWindSystem(float DeltaTime);
    void UpdateMaterialParameters();
    void ProcessLightningStrike(const FLightningStrike& Strike);
    void PlayThunderSound(const FLightningStrike& Strike, float Delay);
    FWeatherConfiguration InterpolateWeatherConfigs(const FWeatherConfiguration& A, const FWeatherConfiguration& B, float Alpha);
    void ConfigureVolumetricClouds(const FWeatherConfiguration& Config);
    void ConfigureFogSettings(const FWeatherConfiguration& Config);
    void ConfigureLightingSettings(const FWeatherConfiguration& Config);
    void ConfigureAtmosphereSettings(const FWeatherConfiguration& Config);
    FVector CalculateWindVector(float Time) const;
    void SpawnLightningEffect(const FVector& Location, float Intensity);
};