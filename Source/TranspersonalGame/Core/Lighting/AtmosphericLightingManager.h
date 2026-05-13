#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "AtmosphericLightingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 30.0f; // Real minutes per game day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float AtmosphereScale = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 28.0f; // Celsius - Cretaceous tropical
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

    // Time of day system
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetTimeOfDay() const { return TimeSettings.CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayDuration(float Minutes);

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWeatherType NewWeather, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Weather")
    EWeatherType GetCurrentWeather() const { return WeatherState.CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetCloudCoverage(float Coverage);

    // Lighting control
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateFog();

    // Cretaceous period presets
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyCretaceousPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void ApplyTropicalLighting();

    // Manual lighting fixes
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void FixAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ValidateLightingSetup();

protected:
    // Timer functions
    void UpdateTimeOfDay();
    void UpdateWeatherTransition();

    // Component references
    void CacheLightingComponents();
    void FindDirectionalLight();
    void FindSkyAtmosphere();
    void FindVolumetricClouds();
    void FindExponentialFog();

    // Lighting calculations
    FRotator CalculateSunRotation(float TimeOfDay) const;
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    float CalculateSunIntensity(float TimeOfDay) const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    FLight_TimeOfDaySettings TimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather", meta = (AllowPrivateAccess = "true"))
    FLight_WeatherState WeatherState;

    // Component references
    UPROPERTY()
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY()
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY()
    TObjectPtr<AVolumetricCloud> VolumetricClouds;

    UPROPERTY()
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Timer handles
    FTimerHandle TimeUpdateTimer;
    FTimerHandle WeatherUpdateTimer;

    // State tracking
    bool bComponentsCached = false;
    bool bTimeSystemActive = true;
    float WeatherTransitionProgress = 0.0f;
    EWeatherType TargetWeather = EWeatherType::Clear;
};