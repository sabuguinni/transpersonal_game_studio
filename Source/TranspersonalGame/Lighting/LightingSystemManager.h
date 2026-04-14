#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "LightingSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyTint = FLinearColor(0.331f, 0.558f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericDensity = 0.8f;

    FLight_TimeOfDaySettings()
    {
        TimeOfDay = 12.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunIntensity = 5.0f;
        SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
        SkyTint = FLinearColor(0.331f, 0.558f, 1.0f, 1.0f);
        AtmosphericDensity = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;

    FLight_WeatherSettings()
    {
        WeatherType = EWeatherType::Clear;
        CloudCoverage = 0.3f;
        FogDensity = 0.02f;
        RainIntensity = 0.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        WindStrength = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Time of day system
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StartDayNightCycle(float CycleDurationMinutes = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StopDayNightCycle();

    UFUNCTION(BlueprintPure, Category = "Lighting")
    bool IsDayNightCycleActive() const { return bDayNightCycleActive; }

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EWeatherType NewWeatherType);

    UFUNCTION(BlueprintPure, Category = "Weather")
    EWeatherType GetCurrentWeatherType() const { return CurrentWeatherSettings.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EWeatherType TargetWeather, float TransitionDuration = 5.0f);

    // Lighting configuration
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigurePrehistoricAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupVolumetricLighting();

    // Interior lighting
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureInteriorLighting(AActor* InteriorSpace, float AmbientIntensity = 0.3f);

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, NewTimeOfDay);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EWeatherType, NewWeatherType);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWeatherChanged OnWeatherChanged;

protected:
    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FLight_TimeOfDaySettings CurrentTimeSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FLight_WeatherSettings CurrentWeatherSettings;

    // Day/night cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    bool bDayNightCycleActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float DayNightCycleDuration = 1200.0f; // 20 minutes in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float TimeSpeed = 1.0f;

    // Lighting actors
    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    ASkyLight* SkyLight;

    UPROPERTY()
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    AVolumetricCloud* VolumetricClouds;

    // Timer handles
    FTimerHandle DayNightCycleTimer;
    FTimerHandle WeatherTransitionTimer;

    // Internal methods
    void UpdateTimeOfDay();
    void UpdateSunPosition();
    void UpdateSunColor();
    void UpdateAtmosphere();
    void UpdateWeatherEffects();
    void FindOrCreateLightingActors();
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    FRotator CalculateSunRotation(float TimeOfDay) const;
    float CalculateSunIntensity(float TimeOfDay) const;

private:
    // Weather transition
    FLight_WeatherSettings TargetWeatherSettings;
    float WeatherTransitionProgress = 0.0f;
    float WeatherTransitionDuration = 5.0f;
    bool bWeatherTransitioning = false;

    void UpdateWeatherTransition();
    FLight_WeatherSettings LerpWeatherSettings(const FLight_WeatherSettings& A, const FLight_WeatherSettings& B, float Alpha) const;
};