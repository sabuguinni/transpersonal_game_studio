#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "LightingSystemManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Sunset      UMETA(DisplayName = "Sunset"),
    Twilight    UMETA(DisplayName = "Twilight"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FLight_TimeOfDaySettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunAngle = 45.0f;
        RayleighScattering = 0.0331f;
        SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereIntensity = 1.0f;

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.3f;
        CloudOpacity = 0.8f;
        FogHeightFalloff = 0.2f;
        FogMaxOpacity = 1.0f;
        AtmosphereIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_LightingSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_LightingSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class AVolumetricCloud* VolumetricClouds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class AExponentialHeightFog* HeightFog;

    // Time and weather state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeHours = 12.0f;

    // Lighting presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherState, FLight_WeatherSettings> WeatherPresets;

    // Dynamic lighting control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    bool bEnableDynamicTimeOfDay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    bool bEnableDynamicWeather = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float WeatherTransitionSpeed = 0.5f;

    // Cretaceous atmosphere settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous")
    bool bUseCretaceousAtmosphere = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous")
    float CretaceousTemperatureMultiplier = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous")
    float CretaceousHumidityLevel = 0.8f;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherState(ELight_WeatherState NewWeatherState, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDayByHours(float Hours, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ResetToDefaultLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void InitializeLightingComponents();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_TimeOfDaySettings GetCurrentTimeOfDaySettings() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLight_WeatherSettings GetCurrentWeatherSettings() const;

protected:
    // Internal update methods
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings, bool bInstant = false);
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings, bool bInstant = false);
    void InterpolateLightingSettings(float Alpha);
    
    // Component management
    void FindLightingComponents();
    void ValidateLightingComponents();
    void CreateMissingComponents();
    
    // Preset initialization
    void InitializeTimeOfDayPresets();
    void InitializeWeatherPresets();
    
    // Cretaceous-specific methods
    void SetupCretaceousDefaults();
    void ApplyCretaceousColorGrading();
    void ConfigureCretaceousAtmosphere();

private:
    // Transition state
    FLight_TimeOfDaySettings PreviousTimeSettings;
    FLight_TimeOfDaySettings TargetTimeSettings;
    FLight_WeatherSettings PreviousWeatherSettings;
    FLight_WeatherSettings TargetWeatherSettings;
    
    float TimeTransitionAlpha = 0.0f;
    float WeatherTransitionAlpha = 0.0f;
    bool bIsTransitioning = false;
    
    // Component validation flags
    bool bComponentsInitialized = false;
    bool bPresetsInitialized = false;
};