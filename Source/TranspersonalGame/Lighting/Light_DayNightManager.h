#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_DayNightManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn       UMETA(DisplayName = "Dawn"),
    Morning    UMETA(DisplayName = "Morning"),
    Noon       UMETA(DisplayName = "Noon"),
    Afternoon  UMETA(DisplayName = "Afternoon"),
    Dusk       UMETA(DisplayName = "Dusk"),
    Night      UMETA(DisplayName = "Night"),
    Midnight   UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear      UMETA(DisplayName = "Clear"),
    Cloudy     UMETA(DisplayName = "Cloudy"),
    Overcast   UMETA(DisplayName = "Overcast"),
    Rain       UMETA(DisplayName = "Rain"),
    Storm      UMETA(DisplayName = "Storm"),
    Fog        UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-45.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float AtmosphereIntensity = 1.0f;

    FLight_TimeSettings()
    {
        SunIntensity = 8.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        AtmosphereIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor WeatherTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightningChance = 0.0f;

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.5f;
        RainIntensity = 0.0f;
        WindStrength = 1.0f;
        WeatherTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        LightningChance = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DayNightManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_DayNightManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Day/Night Cycle Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bSmoothTransitions = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherState CurrentWeather = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float WeatherChangeIntervalMinutes = 10.0f;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* MainSunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* GlobalFogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class APostProcessVolume* PostProcessVolume;

    // Time Settings for Different Periods
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeSettings DawnSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeSettings NoonSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeSettings DuskSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLight_TimeSettings NightSettings;

    // Weather Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLight_WeatherSettings ClearWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLight_WeatherSettings CloudyWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLight_WeatherSettings RainWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLight_WeatherSettings StormWeather;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimeOfDayEnum() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerWeatherTransition();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor)
    void InitializeLightingSystem();

private:
    // Internal state
    float WeatherTimer = 0.0f;
    float TransitionSpeed = 1.0f;
    bool bIsTransitioning = false;
    
    // Internal functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void InterpolateLightingSettings(float Alpha, const FLight_TimeSettings& From, const FLight_TimeSettings& To);
    FLight_TimeSettings GetCurrentTimeSettings() const;
    FLight_WeatherSettings GetCurrentWeatherSettings() const;
    void ValidateLightingReferences();
};