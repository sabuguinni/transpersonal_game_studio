#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "AtmosphericLightingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Hour = 12.0f; // 0-24 hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float Minute = 0.0f; // 0-60 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeSpeed = 1.0f; // Multiplier for time progression

    FLight_TimeOfDay()
    {
        Hour = 12.0f;
        Minute = 0.0f;
        TimeSpeed = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f; // 0-1 cloud density

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f; // 0-1 rain strength

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.1f; // 0-1 fog thickness

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f; // 0-1 wind power

    FLight_WeatherState()
    {
        CloudCoverage = 0.3f;
        RainIntensity = 0.0f;
        FogDensity = 0.1f;
        WindStrength = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunAngle = 45.0f; // Elevation angle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.004f;

    FLight_AtmosphericSettings()
    {
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunIntensity = 5.0f;
        SunAngle = 45.0f;
        RayleighScattering = 0.0331f;
        MieScattering = 0.004f;
    }
};

/**
 * Atmospheric Lighting Manager - Handles day/night cycle, weather systems, and atmospheric lighting
 * for the Cretaceous period prehistoric world. Manages dynamic lighting that responds to time,
 * weather conditions, and environmental factors to create immersive atmospheric conditions.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAtmosphericLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAtmosphericLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Time Management
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(float Hour, float Minute);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    FLight_TimeOfDay GetCurrentTime() const { return CurrentTime; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void PauseTimeProgression(bool bPause);

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeatherState(const FLight_WeatherState& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    FLight_WeatherState GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void TransitionToWeather(const FLight_WeatherState& TargetWeather, float TransitionTime);

    // Atmospheric Settings
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetCretaceousDefaults();

    // Component Management
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void RefreshLightingComponents();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyLightingPreset(const FString& PresetName);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void TriggerLightningStrike(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetVolumetricFogEnabled(bool bEnabled);

protected:
    // Core time and weather state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FLight_TimeOfDay CurrentTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FLight_WeatherState CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FLight_AtmosphericSettings AtmosphericSettings;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class AVolumetricCloud> VolumetricClouds;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<class AExponentialHeightFog> HeightFog;

    // Weather transition
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    FLight_WeatherState TargetWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherTransitionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherTransitionProgress = 0.0f;

    // Control flags
    UPROPERTY(BlueprintReadOnly, Category = "Control")
    bool bTimePaused = false;

    UPROPERTY(BlueprintReadOnly, Category = "Control")
    bool bWeatherTransitioning = false;

private:
    // Internal update methods
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateSkyAtmosphere();
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateVolumetricClouds();
    void UpdateHeightFog();

    // Component discovery
    void FindLightingComponents();
    void ValidateComponents();

    // Utility methods
    float CalculateSunAngle() const;
    FLinearColor CalculateSunColor() const;
    float CalculateSunIntensity() const;
    FLinearColor InterpolateWeatherColor(const FLinearColor& Clear, const FLinearColor& Stormy, float Factor) const;
};