#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "VFX_WeatherController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear UMETA(DisplayName = "Clear Sky"),
    Rain_Light UMETA(DisplayName = "Light Rain"),
    Rain_Heavy UMETA(DisplayName = "Heavy Rain"),
    Snow_Light UMETA(DisplayName = "Light Snow"),
    Snow_Heavy UMETA(DisplayName = "Heavy Snow"),
    Fog_Light UMETA(DisplayName = "Light Fog"),
    Fog_Heavy UMETA(DisplayName = "Heavy Fog"),
    Storm UMETA(DisplayName = "Thunder Storm")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor LightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float LightIntensity = 1.0f;

    FVFX_WeatherSettings()
    {
        WeatherType = EVFX_WeatherType::Clear;
        Intensity = 1.0f;
        Duration = 60.0f;
        LightColor = FLinearColor::White;
        LightIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_WeatherController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_WeatherController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* WeatherParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    EVFX_WeatherType CurrentWeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TArray<FVFX_WeatherSettings> WeatherConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    bool bAutoChangeWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float WeatherChangeInterval = 300.0f; // 5 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float TransitionDuration = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void ChangeWeather(EVFX_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void StartWeatherTransition(EVFX_WeatherType TargetWeather, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    EVFX_WeatherType GetRandomWeatherType() const;

private:
    float CurrentWeatherTimer = 0.0f;
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDurationCurrent = 30.0f;
    EVFX_WeatherType TransitionTargetWeather = EVFX_WeatherType::Clear;

    void UpdateWeatherSystem();
    void ProcessWeatherTransition(float DeltaTime);
    FVFX_WeatherSettings GetWeatherSettings(EVFX_WeatherType WeatherType) const;
    void ApplyWeatherSettings(const FVFX_WeatherSettings& Settings);
    void UpdateDirectionalLight(const FVFX_WeatherSettings& Settings);
};