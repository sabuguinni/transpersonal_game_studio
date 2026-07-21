#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "AtmosphericLightingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float AtmosphereScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bIsRaining = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphericLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphericLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UDirectionalLightComponent* SunLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class USkyAtmosphereComponent* AtmosphereComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UVolumetricCloudComponent* CloudComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class UExponentialHeightFogComponent* FogComponent;

    // Time and weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f; // Real-time minutes for full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_TimeOfDaySettings NightSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FLight_WeatherSettings CurrentWeather;

    // Interior lighting support
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    TArray<class APointLight*> InteriorLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    float InteriorLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Lighting")
    FLinearColor FireLightColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

public:
    // Day/Night cycle functions
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    float GetTimeOfDay() const { return TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetDayNightCycleEnabled(bool bEnabled);

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherSettings(const FLight_WeatherSettings& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartRain(float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StopRain();

    // Interior lighting functions
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void RegisterInteriorLight(class APointLight* Light);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void UpdateInteriorLighting();

    // Atmospheric correction functions
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Correction")
    void ApplyCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Correction")
    void FixAtmosphericSettings();

private:
    // Internal update functions
    void UpdateSunPosition();
    void UpdateAtmosphereSettings();
    void UpdateFogSettings();
    void UpdateCloudSettings();
    void InterpolateLightingSettings(float Alpha, const FLight_TimeOfDaySettings& SettingsA, const FLight_TimeOfDaySettings& SettingsB);

    // Helper functions
    float GetSunAngleFromTime(float Time) const;
    FLight_TimeOfDaySettings GetCurrentTimeSettings() const;
    void FindAndCacheLightingActors();

    // Cached references to world lighting actors
    UPROPERTY()
    class ADirectionalLight* WorldSun;

    UPROPERTY()
    class ASkyAtmosphere* WorldAtmosphere;

    UPROPERTY()
    class AVolumetricCloud* WorldClouds;

    UPROPERTY()
    class AExponentialHeightFog* WorldFog;
};