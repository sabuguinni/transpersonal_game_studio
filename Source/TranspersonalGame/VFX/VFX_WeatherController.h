#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "NiagaraComponent.h"
#include "VFX_WeatherController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear,
    LightRain,
    HeavyRain,
    Storm,
    Fog,
    Dust,
    Snow
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Visibility = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Components")
    class UNiagaraComponent* RainParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Components")
    class UNiagaraComponent* FogParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Components")
    class USkyLightComponent* SkyLightComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FVFX_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<EVFX_WeatherType, class UNiagaraSystem*> WeatherParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    bool bAutoWeatherCycle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings", meta = (EditCondition = "bAutoWeatherCycle"))
    float WeatherCycleDuration = 300.0f; // 5 minutes

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(EVFX_WeatherType NewWeatherType, float Intensity = 0.5f, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void TransitionToWeather(EVFX_WeatherType TargetWeatherType, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeatherSettings.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    float GetWeatherIntensity() const { return CurrentWeatherSettings.Intensity; }

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StartWeatherCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StopWeatherCycle();

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherChanged(EVFX_WeatherType NewWeatherType, float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionStarted(EVFX_WeatherType FromWeather, EVFX_WeatherType ToWeather);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Events")
    void OnWeatherTransitionCompleted(EVFX_WeatherType NewWeatherType);

private:
    void InitializeWeatherSystems();
    void UpdateWeatherEffects();
    void UpdateLighting();
    void UpdateParticleEffects();
    void ProcessWeatherTransition(float DeltaTime);
    void CycleWeather();

    // Transition state
    bool bIsTransitioning;
    EVFX_WeatherType TransitionFromWeather;
    EVFX_WeatherType TransitionToWeather;
    float TransitionTimer;
    float TransitionDuration;

    // Weather cycle state
    float WeatherCycleTimer;
    int32 CurrentWeatherIndex;
    TArray<EVFX_WeatherType> WeatherCycleSequence;
};