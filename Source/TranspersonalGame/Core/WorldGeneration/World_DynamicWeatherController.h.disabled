#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/WindDirectionalSource.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "SharedTypes.h"
#include "World_DynamicWeatherController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType CurrentWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity = 3.0f;

    FWorld_WeatherState()
    {
        CurrentWeather = EWorld_WeatherType::Clear;
        WeatherIntensity = 0.5f;
        TransitionSpeed = 1.0f;
        WindStrength = 0.3f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        FogDensity = 0.02f;
        CloudCoverage = 0.4f;
        SunIntensity = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EWorld_WeatherType TargetWeather = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float CurrentProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bIsTransitioning = false;

    FWorld_WeatherTransition()
    {
        TargetWeather = EWorld_WeatherType::Clear;
        TransitionDuration = 30.0f;
        CurrentProgress = 0.0f;
        bIsTransitioning = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_DynamicWeatherController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicWeatherController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State", meta = (AllowPrivateAccess = "true"))
    FWorld_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State", meta = (AllowPrivateAccess = "true"))
    FWorld_WeatherTransition WeatherTransition;

    // Weather Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    bool bEnableAutomaticWeatherChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    float WeatherChangeInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control")
    float TimeSinceLastWeatherChange = 0.0f;

    // Environmental References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TSoftObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TSoftObjectPtr<AWindDirectionalSource> WindActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TSoftObjectPtr<ADirectionalLight> SunActor;

    // Weather Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Presets")
    TMap<EWorld_WeatherType, FWorld_WeatherState> WeatherPresets;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(EWorld_WeatherType NewWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherImmediate(EWorld_WeatherType NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    EWorld_WeatherType GetCurrentWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    float GetWeatherIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    bool IsWeatherTransitioning() const;

    // Environmental Control
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateWindSettings();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateLightingSettings();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateCloudSettings();

    // Weather Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherChanged(EWorld_WeatherType OldWeather, EWorld_WeatherType NewWeather);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionStarted(EWorld_WeatherType TargetWeather);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionCompleted(EWorld_WeatherType NewWeather);

protected:
    // Internal Functions
    void InitializeWeatherPresets();
    void FindEnvironmentalActors();
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherState(const FWorld_WeatherState& WeatherState);
    FWorld_WeatherState InterpolateWeatherStates(const FWorld_WeatherState& From, const FWorld_WeatherState& To, float Alpha);
    EWorld_WeatherType GetRandomWeatherType() const;
    void TriggerAutomaticWeatherChange();

    // Utility Functions
    float GetWeatherTransitionAlpha() const;
    bool IsValidWeatherType(EWorld_WeatherType WeatherType) const;
    void LogWeatherChange(EWorld_WeatherType OldWeather, EWorld_WeatherType NewWeather) const;
};