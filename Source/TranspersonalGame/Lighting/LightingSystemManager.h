#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "LightingSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeHours = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 20.0f; // Real minutes for full day cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensityDay = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensityNight = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColorDay = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColorNight = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    bool bCloudsEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
    bool bRainEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
    float RainIntensity = 0.5f;
};

UCLASS()
class TRANSPERSONALGAME_API ULightingSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Time of day system
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetTimeOfDay() const { return TimeSettings.CurrentTimeHours; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayDuration(float Minutes);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableDayNightCycle(bool bEnable);

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWeatherType GetCurrentWeather() const { return WeatherSettings.CurrentWeather; }

    // Lighting controls
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetAtmosphereSettings(float ScatteringScale);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetFogSettings(float Density, FLinearColor Color);

    // Cretaceous period presets
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyTropicalAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyJungleLighting();

protected:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return bDayNightCycleEnabled; }
    virtual TStatId GetStatId() const override;

private:
    // Core lighting actors
    UPROPERTY()
    ADirectionalLight* SunActor;

    UPROPERTY()
    AActor* SkyAtmosphereActor;

    UPROPERTY()
    AActor* VolumetricCloudActor;

    UPROPERTY()
    AActor* FogActor;

    // Settings
    UPROPERTY(EditAnywhere, Category = "Settings")
    FLight_TimeOfDaySettings TimeSettings;

    UPROPERTY(EditAnywhere, Category = "Settings")
    FLight_WeatherSettings WeatherSettings;

    // Runtime state
    bool bDayNightCycleEnabled = false;
    float TimeAccumulator = 0.0f;

    // Internal methods
    void FindLightingActors();
    void UpdateSunPosition();
    void UpdateSunLighting();
    void UpdateAtmosphere();
    void UpdateWeatherEffects();
    void ApplyLightingSettings();
};