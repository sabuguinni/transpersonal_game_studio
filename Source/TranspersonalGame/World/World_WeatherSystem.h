#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TimerManager.h"
#include "World_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm")
};

UENUM(BlueprintType)
enum class EWorld_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FWorld_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Visibility = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float FogDensity = 0.02f;

    FWorld_WeatherSettings()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 0.5f;
        Visibility = 1.0f;
        SkyTint = FLinearColor::White;
        FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        FogDensity = 0.02f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTime = 12.0f; // Hours (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float TimeMultiplier = 1.0f; // Speed of day/night cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bEnableDayNightCycle = true;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FWorld_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float WeatherTransitionTime = 300.0f; // Seconds

    // Lighting References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class AExponentialHeightFog* FogActor;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWorld_WeatherType NewWeatherType, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time")
    EWorld_TimeOfDay GetTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRandomWeatherCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopWeatherCycle();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Setup")
    void CreateDefaultLighting();

private:
    // Internal functions
    void UpdateSunPosition();
    void UpdateLighting();
    void UpdateWeatherEffects();
    void TransitionToWeather(EWorld_WeatherType TargetWeather);
    
    // Weather transition
    FTimerHandle WeatherTransitionTimer;
    EWorld_WeatherType TargetWeatherType;
    FWorld_WeatherSettings StartWeatherSettings;
    FWorld_WeatherSettings TargetWeatherSettings;
    float WeatherTransitionProgress = 0.0f;
    bool bIsTransitioning = false;

    // Random weather
    FTimerHandle RandomWeatherTimer;
    bool bRandomWeatherEnabled = false;

    // Helper functions
    FLinearColor GetSunColor(float TimeHours) const;
    float GetSunIntensity(float TimeHours) const;
    FRotator GetSunRotation(float TimeHours) const;
    FWorld_WeatherSettings GetWeatherPreset(EWorld_WeatherType WeatherType) const;
};