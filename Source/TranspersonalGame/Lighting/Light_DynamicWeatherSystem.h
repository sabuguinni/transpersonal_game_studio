#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "TimerManager.h"
#include "Light_DynamicWeatherSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"), 
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Rain        UMETA(DisplayName = "Rain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float AtmosphericPerspective = 1.0f;

    FLight_WeatherSettings()
    {
        SunIntensity = 5.0f;
        SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
        CloudCoverage = 0.3f;
        AtmosphericPerspective = 1.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    ELight_WeatherState CurrentWeatherState = ELight_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherTransitionDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherChangeProbability = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bAutoWeatherChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    float WeatherUpdateInterval = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<ELight_WeatherState, FLight_WeatherSettings> WeatherPresets;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<AExponentialHeightFog> AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTransitionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_WeatherState TargetWeatherState = ELight_WeatherState::Clear;

    FLight_WeatherSettings StartWeatherSettings;
    FLight_WeatherSettings TargetWeatherSettings;
    FTimerHandle WeatherUpdateTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(ELight_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeatherState(ELight_WeatherState NewWeatherState, float TransitionTime = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    ELight_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetAutoWeatherChanges(bool bEnabled) { bAutoWeatherChanges = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void ForceWeatherUpdate();

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherStateChanged(ELight_WeatherState OldState, ELight_WeatherState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionStarted(ELight_WeatherState FromState, ELight_WeatherState ToState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionCompleted(ELight_WeatherState NewState);

protected:
    UFUNCTION()
    void UpdateWeatherSystem();

    UFUNCTION()
    void ProcessWeatherTransition(float DeltaTime);

    UFUNCTION()
    void ApplyWeatherSettings(const FLight_WeatherSettings& Settings);

    UFUNCTION()
    void FindLightingActors();

    UFUNCTION()
    void InitializeWeatherPresets();

    UFUNCTION()
    ELight_WeatherState SelectRandomWeatherState();

    UFUNCTION()
    FLight_WeatherSettings LerpWeatherSettings(const FLight_WeatherSettings& A, const FLight_WeatherSettings& B, float Alpha);
};