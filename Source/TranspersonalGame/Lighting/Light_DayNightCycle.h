#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Light_DayNightCycle.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, NewTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayNightTransition, bool, bIsNight);

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType WeatherType = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor WeatherTint = FLinearColor::White;

    FLight_WeatherSettings()
    {
        WeatherType = ELight_WeatherType::Clear;
        CloudCoverage = 0.0f;
        RainIntensity = 0.0f;
        FogDensityMultiplier = 1.0f;
        LightIntensityMultiplier = 1.0f;
        WeatherTint = FLinearColor::White;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ULight_DayNightCycle : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDayNightTransition OnDayNightTransition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float TimeOfDaySpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bRealTimeMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FLight_WeatherSettings CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableWeatherSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionSpeed = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Time System")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Time System")
    bool IsNightTime() const;

    UFUNCTION(BlueprintCallable, Category = "Time System")
    bool IsDayTime() const;

    UFUNCTION(BlueprintCallable, Category = "Time System")
    FString GetTimeOfDayString() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(ELight_WeatherType NewWeatherType, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    ELight_WeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetSunAngle() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    FLinearColor GetCurrentSkyColor() const;

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    float GetCurrentLightIntensity() const;

private:
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherSystem(float DeltaTime);
    void BroadcastTimeEvents();
    void InitializeAtmosphericSystem();

    UPROPERTY()
    class ALight_AtmosphericSystem* AtmosphericSystem;

    bool bWasNightTime = false;
    float LastBroadcastTime = 0.0f;

    // Weather transition
    FLight_WeatherSettings TargetWeather;
    float WeatherTransitionTimer = 0.0f;
    float WeatherTransitionDuration = 0.0f;
    bool bWeatherTransitioning = false;
};