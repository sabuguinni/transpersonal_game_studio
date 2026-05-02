#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Midnight    UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Rain        UMETA(DisplayName = "Rain")
};

USTRUCT(BlueprintType)
struct FLight_TimeOfDaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    FLight_TimeOfDaySettings()
    {
        SunElevation = 45.0f;
        SunAzimuth = 180.0f;
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SkyLightIntensity = 1.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor AtmosphereColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.5f;
        FogIntensity = 1.0f;
        RainIntensity = 0.0f;
        WindStrength = 0.5f;
        AtmosphereColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULightingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Time of day system
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayNightCycleSpeed(float Speed);

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType NewWeather, float TransitionTime = 5.0f);

    // Lighting configuration
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericLighting();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeatherType; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetDayProgress() const { return DayProgress; }

protected:
    // Core lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<AExponentialHeightFog> AtmosphericFog;

    // Time of day settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    TMap<ELight_TimeOfDay, FLight_TimeOfDaySettings> TimeOfDaySettings;

    // Weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherSettings;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_WeatherType CurrentWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DayProgress;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DayNightCycleSpeed;

    // Weather transition
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    bool bIsTransitioningWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    ELight_WeatherType TargetWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherTransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float WeatherTransitionDuration;

private:
    void FindLightingActors();
    void InitializeTimeOfDaySettings();
    void InitializeWeatherSettings();
    void ApplyTimeOfDaySettings(const FLight_TimeOfDaySettings& Settings);
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);
    void InterpolateWeatherSettings(const FLight_WeatherSettings& From, const FLight_WeatherSettings& To, float Alpha);
    FRotator CalculateSunRotation(float Elevation, float Azimuth) const;
};