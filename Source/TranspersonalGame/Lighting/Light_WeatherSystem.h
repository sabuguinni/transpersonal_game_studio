#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Light_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear UMETA(DisplayName = "Clear Sky"),
    Overcast UMETA(DisplayName = "Overcast"),
    Storm UMETA(DisplayName = "Storm"),
    Fog UMETA(DisplayName = "Heavy Fog"),
    Dawn UMETA(DisplayName = "Dawn"),
    Dusk UMETA(DisplayName = "Dusk")
};

USTRUCT(BlueprintType)
struct FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    FLight_WeatherSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
        CloudCoverage = 0.3f;
        WindStrength = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_WeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    ELight_WeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }

    // Time of Day
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetTimeOfDay(float Hours); // 0-24 hours

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetTimeOfDay() const { return TimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetDayDuration(float Minutes) { DayDurationMinutes = Minutes; }

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Weather Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<ELight_WeatherType, FLight_WeatherSettings> WeatherPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherType CurrentWeatherType = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bAutoWeatherTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float WeatherTransitionIntervalMinutes = 15.0f;

    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay = 12.0f; // 12:00 noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "1.0", ClampMax = "120.0"))
    float DayDurationMinutes = 20.0f; // 20 minutes = 1 full day

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoTimeProgression = true;

    // Transition System
    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    ELight_WeatherType TransitionTargetWeather;

    // Internal State
    FLight_WeatherSettings CurrentSettings;
    FLight_WeatherSettings TargetSettings;
    float TransitionDuration = 5.0f;
    float TransitionTimer = 0.0f;
    FTimerHandle WeatherTransitionTimerHandle;

    // Component References (found at runtime)
    UPROPERTY()
    class ADirectionalLight* SunLight;

    UPROPERTY()
    class ASkyLight* SkyLightActor;

    UPROPERTY()
    class AExponentialHeightFog* FogActor;

    // Weather System Methods
    void InitializeWeatherPresets();
    void FindLightingComponents();
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void ScheduleNextWeatherTransition();

    // Time-based lighting calculations
    FLinearColor CalculateSunColorForTime(float TimeHours) const;
    float CalculateSunIntensityForTime(float TimeHours) const;
    FRotator CalculateSunRotationForTime(float TimeHours) const;
};

#include "Light_WeatherSystem.generated.h"