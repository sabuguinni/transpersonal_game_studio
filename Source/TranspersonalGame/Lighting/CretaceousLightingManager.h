#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "CretaceousLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Storm       UMETA(DisplayName = "Storm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float Temperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);

    FLight_AtmosphericSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        Temperature = 5500.0f;
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.7f, 0.78f, 1.0f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACretaceousLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ACretaceousLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Lighting System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    ASkyAtmosphere* SkyAtmosphereActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting References")
    AExponentialHeightFog* FogActor;

    // Day/Night Cycle Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    ELight_TimeOfDay CurrentTimePhase = ELight_TimeOfDay::Noon;

    // Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableWeatherTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float WeatherTransitionDurationMinutes = 5.0f;

    // Atmospheric Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    TMap<ELight_TimeOfDay, FLight_AtmosphericSettings> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Presets")
    TMap<ELight_WeatherState, FLight_AtmosphericSettings> WeatherPresets;

    // Current Settings
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FLight_AtmosphericSettings CurrentAtmosphericSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    bool bIsTransitioning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float TransitionProgress = 0.0f;

public:
    // Core Lighting Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Management")
    void InitializeLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Lighting Management")
    void UpdateAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Lighting Management")
    void ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings);

    // Day/Night Cycle Functions
    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void SetTimePhase(ELight_TimeOfDay NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    ELight_TimeOfDay GetCurrentTimePhase() const { return CurrentTimePhase; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    // Weather System Functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void CreateMissingLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void SaveCurrentLightingState();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void LoadLightingPresets();

protected:
    // Internal Functions
    void UpdateDayNightCycle(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void CalculateTimePhase();
    void InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha);
    void ApplySunLightSettings(const FLight_AtmosphericSettings& Settings);
    void ApplyFogSettings(const FLight_AtmosphericSettings& Settings);

private:
    // Transition State
    FLight_AtmosphericSettings TransitionStartSettings;
    FLight_AtmosphericSettings TransitionTargetSettings;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 5.0f;
};