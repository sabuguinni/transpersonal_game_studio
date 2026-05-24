#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "TranspersonalGame.h"
#include "../SharedTypes.h"
#include "DynamicLightingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType WeatherType = EWeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 1.0f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDynamicLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeather(EWeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void EnableDynamicTimeProgression(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetTimeOfDay() const { return CurrentTimeSettings.TimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    EWeatherType GetCurrentWeather() const { return CurrentWeatherSettings.WeatherType; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_TimeOfDaySettings CurrentTimeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FLight_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    bool bDynamicTimeEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    float TimeProgressionSpeed = 1.0f; // Real minutes per game hour

private:
    void UpdateSunPosition();
    void UpdateSkyLighting();
    void UpdateFogSettings();
    void UpdateWeatherEffects();
    void FindLightingActors();

    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    UPROPERTY()
    class AVolumetricCloud* CloudActor;

    UPROPERTY()
    class ASkyAtmosphere* AtmosphereActor;

    float AccumulatedTime = 0.0f;
    bool bActorsFound = false;
};