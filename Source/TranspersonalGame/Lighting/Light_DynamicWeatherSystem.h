#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Light_DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Fog             UMETA(DisplayName = "Fog"),
    Dust            UMETA(DisplayName = "Dust Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FString PresetName;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyAtmosphereMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float VisibilityRange = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 30.0f;

    FLight_WeatherPreset()
    {
        PresetName = "Default";
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_DynamicWeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_DynamicWeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Weather State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bAutoWeatherTransition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
    float WeatherTransitionInterval = 300.0f; // 5 minutes

    // Weather Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<ELight_WeatherState, FLight_WeatherPreset> WeatherPresets;

    // Lighting References
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    class ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Atmosphere")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "Sky")
    class ASkyAtmosphere* SkyAtmosphere;

    // Transition State
    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    float TransitionProgress = 0.0f;

    FLight_WeatherPreset CurrentPreset;
    FLight_WeatherPreset TargetPreset;
    FTimerHandle WeatherTransitionTimer;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(ELight_WeatherState NewWeatherState, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartWeatherTransition(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetAutoWeatherTransition(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Weather System")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    float GetTransitionProgress() const { return TransitionProgress; }

    UFUNCTION(BlueprintPure, Category = "Weather System")
    bool IsTransitioning() const { return bIsTransitioning; }

    // Preset Management
    UFUNCTION(BlueprintCallable, Category = "Weather Presets")
    void LoadWeatherPreset(ELight_WeatherState WeatherState, const FLight_WeatherPreset& Preset);

    UFUNCTION(BlueprintPure, Category = "Weather Presets")
    FLight_WeatherPreset GetWeatherPreset(ELight_WeatherState WeatherState) const;

    // Time of Day Integration
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float TimeInHours); // 0.0 = midnight, 12.0 = noon

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void StartDayNightCycle(float DayDurationMinutes = 20.0f);

protected:
    // Internal Functions
    void InitializeWeatherPresets();
    void FindLightingActors();
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherPreset(const FLight_WeatherPreset& Preset);
    void InterpolateWeatherPresets(const FLight_WeatherPreset& From, const FLight_WeatherPreset& To, float Alpha);
    void TriggerRandomWeatherTransition();

    // Day/Night Cycle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 12.0f; // Hours (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayNightCycleSpeed = 1.0f; // Multiplier for time progression

    void UpdateDayNightCycle(float DeltaTime);
    void UpdateSunPosition();
    FLinearColor CalculateSunColorForTime(float TimeInHours);
    float CalculateSunIntensityForTime(float TimeInHours);
};