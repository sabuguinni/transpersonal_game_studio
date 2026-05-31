#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "World_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
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
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity;

    FWorld_WeatherState()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 1.0f;
        Duration = 300.0f; // 5 minutes
        SkyColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        SunIntensity = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_DayNightCycle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float DayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    EWorld_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    float SunAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night")
    FLinearColor SunColor;

    FWorld_DayNightCycle()
    {
        DayDuration = 1200.0f; // 20 minutes real time = 1 game day
        CurrentTime = 0.5f; // Start at noon
        TimeOfDay = EWorld_TimeOfDay::Noon;
        SunAngle = 0.0f;
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    }
};

UCLASS()
class TRANSPERSONALGAME_API UWorld_WeatherSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EWorld_WeatherType NewWeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetTimeOfDay(float TimeNormalized);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_WeatherType GetCurrentWeather() const { return CurrentWeatherState.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    EWorld_TimeOfDay GetCurrentTimeOfDay() const { return DayNightCycle.TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    float GetDayProgress() const { return DayNightCycle.CurrentTime; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartWeatherTransition(EWorld_WeatherType TargetWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetDayDuration(float NewDuration) { DayNightCycle.DayDuration = NewDuration; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Day/Night")
    FWorld_DayNightCycle DayNightCycle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    bool bAutoWeatherChanges;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    float WeatherChangeInterval;

    UPROPERTY()
    ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY()
    class AExponentialHeightFog* HeightFog;

private:
    float WeatherTimer;
    bool bIsTransitioning;
    FWorld_WeatherState TargetWeatherState;
    float TransitionTimer;
    float TransitionDuration;

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherEffects(float DeltaTime);
    void UpdateSunLight();
    void UpdateSkyAtmosphere();
    void UpdateFog();
    void FindEnvironmentActors();
    EWorld_TimeOfDay CalculateTimeOfDay(float TimeNormalized);
    FLinearColor GetSunColorForTime(EWorld_TimeOfDay TimeOfDay);
    float GetSunIntensityForTime(EWorld_TimeOfDay TimeOfDay);
    FWorld_WeatherState GetRandomWeatherState();
};