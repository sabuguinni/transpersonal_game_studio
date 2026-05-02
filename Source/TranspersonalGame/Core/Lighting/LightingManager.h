#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn     UMETA(DisplayName = "Dawn"),
    Morning  UMETA(DisplayName = "Morning"), 
    Midday   UMETA(DisplayName = "Midday"),
    Evening  UMETA(DisplayName = "Evening"),
    Dusk     UMETA(DisplayName = "Dusk"),
    Night    UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear    UMETA(DisplayName = "Clear"),
    Cloudy   UMETA(DisplayName = "Cloudy"),
    Rainy    UMETA(DisplayName = "Rainy"),
    Foggy    UMETA(DisplayName = "Foggy"),
    Storm    UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct FLight_TimeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    FLinearColor FogColor = FLinearColor(0.8f, 0.7f, 0.5f);

    FLight_TimeSettings()
    {
        SunAngle = 0.0f;
        SunIntensity = 3.0f;
        SunColor = FLinearColor::White;
        SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.7f, 0.5f);
    }
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FLinearColor AtmosphereColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float VisibilityDistance = 10000.0f;

    FLight_WeatherSettings()
    {
        CloudCoverage = 0.0f;
        RainIntensity = 0.0f;
        WindStrength = 0.0f;
        AtmosphereColor = FLinearColor::White;
        VisibilityDistance = 10000.0f;
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

    // Time of day system
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDaySmooth(ELight_TimeOfDay NewTimeOfDay, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Lighting")
    ELight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StartDayNightCycle(float CycleDurationMinutes = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void StopDayNightCycle();

    // Weather system
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(ELight_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherSmooth(ELight_WeatherType NewWeather, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintPure, Category = "Weather")
    ELight_WeatherType GetCurrentWeather() const { return CurrentWeather; }

    // Manual lighting control
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunAngle(float Angle);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetSunIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphere();

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_TimeOfDay CurrentTimeOfDay = ELight_TimeOfDay::Midday;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_WeatherType CurrentWeather = ELight_WeatherType::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDayNightCycleActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DayNightCycleDuration = 1200.0f; // 20 minutes in seconds

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentCycleTime = 0.0f;

    // Time settings for each time of day
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TMap<ELight_TimeOfDay, FLight_TimeSettings> TimeSettings;

    // Weather settings for each weather type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherSettings;

    // Actor references
    UPROPERTY()
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY()
    TObjectPtr<class ASkyLight> SkyLight;

    UPROPERTY()
    TObjectPtr<class AExponentialHeightFog> HeightFog;

    // Internal methods
    void FindLightingActors();
    void InitializeTimeSettings();
    void InitializeWeatherSettings();
    void ApplyTimeSettings(const FLight_TimeSettings& Settings);
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);
    void UpdateDayNightCycle(float DeltaTime);

    // Timer handle for day/night cycle
    FTimerHandle DayNightCycleTimer;
};