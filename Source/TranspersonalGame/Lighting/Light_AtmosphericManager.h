#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "../SharedTypes.h"
#include "Light_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f;

    FLight_WeatherState()
    {
        CloudCoverage = 0.3f;
        FogDensity = 0.02f;
        RainIntensity = 0.0f;
        SkyTint = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        WindStrength = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentHour = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDuration = 1200.0f; // 20 minutes real time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float MoonIntensity = 0.1f;

    FLight_TimeOfDay()
    {
        CurrentHour = 12.0f;
        DayDuration = 1200.0f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        MoonIntensity = 0.1f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ALight_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericManager();

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog")
    class UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clouds")
    class UVolumetricCloudComponent* VolumetricClouds;

    // State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    FLight_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FLight_WeatherState CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FLight_WeatherState TargetWeather;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float WeatherTransitionSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FLight_WeatherState> WeatherPresets;

    // Biome-specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FLight_WeatherState> BiomeWeatherSettings;

public:
    // Time Control
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    float GetTimeOfDay() const { return TimeOfDay.CurrentHour; }

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetDayDuration(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    bool IsNightTime() const { return TimeOfDay.CurrentHour < 6.0f || TimeOfDay.CurrentHour > 18.0f; }

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(const FLight_WeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(const FLight_WeatherState& TargetState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FLight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void SetBiomeWeather(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void UpdateWeatherForBiome(EBiomeType BiomeType, const FLight_WeatherState& WeatherState);

    // Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunColor(const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCloudCoverage(float Coverage);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SaveCurrentSettings();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LoadSavedSettings();

private:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateLightingParameters();
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateAtmosphericScattering();
    void UpdateVolumetricClouds();
    void UpdateHeightFog();

    // Helper functions
    FLinearColor CalculateSunColor(float Hour) const;
    float CalculateSunIntensity(float Hour) const;
    FRotator CalculateSunRotation(float Hour) const;

    // Transition state
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bIsTransitioning = false;
};