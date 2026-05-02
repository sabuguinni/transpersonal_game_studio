#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn     UMETA(DisplayName = "Dawn"),
    Morning  UMETA(DisplayName = "Morning"),
    Midday   UMETA(DisplayName = "Midday"),
    Afternoon UMETA(DisplayName = "Afternoon"),
    Dusk     UMETA(DisplayName = "Dusk"),
    Night    UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear    UMETA(DisplayName = "Clear"),
    Cloudy   UMETA(DisplayName = "Cloudy"),
    Overcast UMETA(DisplayName = "Overcast"),
    Rainy    UMETA(DisplayName = "Rainy"),
    Stormy   UMETA(DisplayName = "Stormy"),
    Foggy    UMETA(DisplayName = "Foggy")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaze = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ExposureCompensation = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float WeatherTransitionDuration = 10.0f;

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class ASkyAtmosphere* AtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    class AExponentialHeightFog* FogActor;

    // Time Settings for each period
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    // Weather Settings for each type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherSettings;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherType NewWeather, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void InitializeDefaultSettings();

private:
    void UpdateSunPosition();
    void UpdateSunIntensity();
    void UpdateAtmosphere();
    void UpdateFog();
    void UpdateWeatherEffects();
    
    ELight_TimeOfDay GetTimeOfDayEnum() const;
    FLight_TimeSettings GetCurrentTimeSettings() const;
    FLight_WeatherSettings GetCurrentWeatherSettings() const;
    
    float WeatherTransitionProgress = 0.0f;
    ELight_WeatherType PreviousWeather = ELight_WeatherType::Clear;
    bool bTransitioningWeather = false;
};