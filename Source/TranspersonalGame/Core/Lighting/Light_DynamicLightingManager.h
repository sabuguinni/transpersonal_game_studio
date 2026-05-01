#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Light_DynamicLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn = 0,
    Morning = 1,
    Noon = 2,
    Afternoon = 3,
    Dusk = 4,
    Night = 5
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear = 0,
    Cloudy = 1,
    Rainy = 2,
    Foggy = 3,
    Storm = 4,
    VolcanicAsh = 5
};

USTRUCT(BlueprintType)
struct FLight_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunElevation = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULight_DynamicLightingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetWeatherType(ELight_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetDayNightCycleSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting(float DeltaTime);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Lighting")
    float GetDayProgress() const { return DayProgress; }

protected:
    // Core lighting actors
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLight;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* HeightFog;

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Noon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float DayProgress = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DayNightCycleSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDynamicCycle = true;

    // Lighting presets for different times/weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_TimeOfDay, FLight_LightingSettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<ELight_WeatherType, FLight_LightingSettings> WeatherPresets;

private:
    // Internal functions
    void FindLightingActors();
    void InitializePresets();
    void ApplyLightingSettings(const FLight_LightingSettings& Settings);
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdateFog();
    FLight_LightingSettings GetCurrentLightingSettings() const;
    FLight_LightingSettings BlendLightingSettings(const FLight_LightingSettings& A, const FLight_LightingSettings& B, float Alpha) const;
};