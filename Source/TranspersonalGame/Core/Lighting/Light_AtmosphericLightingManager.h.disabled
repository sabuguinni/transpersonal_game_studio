#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "../SharedTypes.h"
#include "Light_AtmosphericLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FLight_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    FLight_LightingSettings()
    {
        // Default constructor with initialization above
    }
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType WeatherType = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Precipitation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 1.0f;

    FLight_WeatherSettings()
    {
        // Default constructor with initialization above
    }
};

/**
 * Manages atmospheric lighting, day/night cycles, and weather systems for the prehistoric world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphericLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphericLightingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === LIGHTING COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class ASkyLight* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting Components")
    class APostProcessVolume* GlobalPostProcess;

    // === TIME AND WEATHER SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDayHours = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableWeatherSystem = true;

    // === LIGHTING PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_LightingSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_LightingSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_LightingSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Presets")
    FLight_LightingSettings NightSettings;

    // === WEATHER PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FLight_WeatherSettings ClearWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FLight_WeatherSettings CloudyWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    FLight_WeatherSettings StormyWeather;

    // === PUBLIC FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyLightingSettings(const FLight_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    ELight_TimeOfDay GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    float GetDayNightProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void CreateLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateSunPosition();

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateAtmosphericScattering();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugCurrentLighting();

private:
    // === INTERNAL FUNCTIONS ===

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void InterpolateLightingSettings(const FLight_LightingSettings& From, const FLight_LightingSettings& To, float Alpha);
    FLight_LightingSettings GetLightingSettingsForTime(ELight_TimeOfDay TimeOfDay) const;
    FRotator CalculateSunRotationForTime(float Hours) const;

    // === INTERNAL STATE ===

    UPROPERTY()
    FLight_LightingSettings CurrentLightingSettings;

    UPROPERTY()
    FLight_WeatherSettings CurrentWeatherSettings;

    float WeatherTransitionTimer = 0.0f;
    ELight_WeatherType TargetWeather = ELight_WeatherType::Clear;
    bool bIsTransitioningWeather = false;
};