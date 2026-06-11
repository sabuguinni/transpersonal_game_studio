#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "TimerManager.h"
#include "Light_DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Dawn        UMETA(DisplayName = "Dawn"),
    Dusk        UMETA(DisplayName = "Dusk")
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float BloomIntensity = 0.675f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float ExposureBias = 0.5f;

    FLight_WeatherSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);
        BloomIntensity = 0.675f;
        ExposureBias = 0.5f;
    }
};

UCLASS(ClassGroup=(Lighting), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_DynamicWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DynamicWeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(ELight_WeatherState NewState);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherState NewState, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Weather System")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    // Dynamic Weather
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void EnableDynamicWeather(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherTransitionInterval(float MinInterval, float MaxInterval);

    // Time of Day
    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetTimeOfDay(float Hours); // 0-24

    UFUNCTION(BlueprintPure, Category = "Time System")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Time System")
    void SetDayDuration(float DurationInMinutes);

protected:
    // Weather States
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Presets")
    TMap<ELight_WeatherState, FLight_WeatherSettings> WeatherPresets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather System")
    ELight_WeatherState CurrentWeatherState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather System")
    ELight_WeatherState TargetWeatherState;

    // Transition System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather System")
    bool bIsTransitioning;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather System")
    float TransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather System")
    float TransitionDuration;

    // Dynamic Weather
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Weather")
    bool bDynamicWeatherEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Weather")
    float MinWeatherInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Weather")
    float MaxWeatherInterval;

    FTimerHandle WeatherTransitionTimer;

    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float CurrentTimeOfDay; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bTimeProgression;

    // Component References
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY()
    class APostProcessVolume* PostProcessVolume;

private:
    void InitializeWeatherPresets();
    void FindLightingComponents();
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateSunPosition();
    void ScheduleNextWeatherChange();
    void OnWeatherTransitionComplete();

    FLight_WeatherSettings LerpWeatherSettings(const FLight_WeatherSettings& A, const FLight_WeatherSettings& B, float Alpha);
};