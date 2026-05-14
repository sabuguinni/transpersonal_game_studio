#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "AtmosphericLightingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Settings")
    float SunTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Settings")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);

    FLight_TimeOfDaySettings()
    {
        TimeOfDay = 12.0f;
        DayDurationMinutes = 20.0f;
        SunIntensity = 8.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        SunTemperature = 5500.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    FLight_WeatherSettings()
    {
        CurrentWeather = EWeatherType::Clear;
        CloudCoverage = 0.3f;
        Humidity = 0.7f;
        WindStrength = 1.0f;
        RainIntensity = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAtmosphericLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAtmosphericLightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Time of Day System
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    float GetTimeOfDay() const { return TimeSettings.TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetDayDuration(float DurationMinutes);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void StartDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void StopDayNightCycle();

    // Weather System
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeather(EWeatherType NewWeather, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    EWeatherType GetCurrentWeather() const { return WeatherSettings.CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetCloudCoverage(float Coverage);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetFogDensity(float Density);

    // Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ApplyCretaceousAtmosphere();

    // Actor References
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ValidateLightingSetup();

    // Settings Access
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_TimeOfDaySettings TimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_WeatherSettings WeatherSettings;

protected:
    // Internal Update Functions
    void TickDayNightCycle();
    void UpdateSunIntensity();
    void UpdateSunColor();
    void UpdateFogSettings();
    void UpdatePostProcessing();

    // Actor References
    UPROPERTY()
    ADirectionalLight* SunActor;

    UPROPERTY()
    ASkyLight* SkyLightActor;

    UPROPERTY()
    AExponentialHeightFog* FogActor;

    UPROPERTY()
    APostProcessVolume* PostProcessVolume;

    // Timer Management
    FTimerHandle DayNightTimerHandle;
    bool bDayNightCycleActive;

    // Transition State
    bool bInWeatherTransition;
    float WeatherTransitionTime;
    float WeatherTransitionDuration;
    EWeatherType PreviousWeather;

private:
    // Internal Calculations
    float CalculateSunElevation() const;
    FLinearColor CalculateSunColorFromTime() const;
    float CalculateSunIntensityFromTime() const;
    FRotator CalculateSunRotationFromTime() const;
};