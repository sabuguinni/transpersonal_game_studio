#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Particles/ParticleSystemComponent.h"
#include "Light_DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ELight_WeatherType WeatherType = ELight_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    FLight_WeatherPreset()
    {
        WeatherType = ELight_WeatherType::Clear;
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        CloudCoverage = 0.3f;
        FogDensity = 0.1f;
        RainIntensity = 0.0f;
        WindStrength = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_DynamicWeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_DynamicWeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(ELight_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StartRandomWeatherCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void StopWeatherCycle();

    // Time of Day
    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float Hours); // 0-24 hours

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetDayNightCycleSpeed(float SpeedMultiplier);

    // Lighting Control
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetCretaceousAtmosphere();

protected:
    // Weather Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<ELight_WeatherType, FLight_WeatherPreset> WeatherPresets;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ELight_WeatherType CurrentWeatherType = ELight_WeatherType::Clear;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float CurrentTimeOfDay = 12.0f; // Noon

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayNightCycleSpeed = 1.0f;

    // Weather Transition
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionProgress = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    float TransitionDuration = 5.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
    ELight_WeatherType TargetWeatherType = ELight_WeatherType::Clear;

    // Random Weather Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Weather")
    bool bRandomWeatherEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Weather")
    float MinWeatherDuration = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Weather")
    float MaxWeatherDuration = 900.0f; // 15 minutes

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Random Weather")
    float NextWeatherChangeTime = 0.0f;

    // Component References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    ASkyAtmosphere* SkyAtmosphere = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
    APostProcessVolume* PostProcessVolume = nullptr;

    // Helper Functions
    void InitializeWeatherPresets();
    void FindLightingComponents();
    void UpdateSunPosition();
    void UpdateAtmosphere();
    void UpdatePostProcess();
    void TickWeatherTransition(float DeltaTime);
    void TickRandomWeather(float DeltaTime);
    FLight_WeatherPreset GetCurrentWeatherPreset() const;
    FLight_WeatherPreset LerpWeatherPresets(const FLight_WeatherPreset& A, const FLight_WeatherPreset& B, float Alpha) const;
};