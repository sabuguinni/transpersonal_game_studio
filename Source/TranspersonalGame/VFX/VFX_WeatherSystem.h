#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "VFX_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    Snow            UMETA(DisplayName = "Snow"),
    Dust            UMETA(DisplayName = "Dust Storm"),
    Wind            UMETA(DisplayName = "Strong Wind")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float LightIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "50000.0"))
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WindStrength = 10.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_WeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_WeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(EVFX_WeatherType NewWeatherType, float Intensity = 0.5f, float Duration = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TransitionToWeather(const FVFX_WeatherSettings& NewSettings, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StopCurrentWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeatherSettings.WeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetWeatherIntensity() const { return CurrentWeatherSettings.Intensity; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    bool IsWeatherActive() const { return bWeatherActive; }

    // Lightning System
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void TriggerLightning(FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetLightningFrequency(float MinInterval, float MaxInterval);

protected:
    // Weather Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FVFX_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FVFX_WeatherSettings TargetWeatherSettings;

    // Niagara Systems for Different Weather Types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> SnowSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> DustStormSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> LightningSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    TSoftObjectPtr<UNiagaraSystem> WindSystem;

    // Active Weather Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Components")
    TObjectPtr<UNiagaraComponent> ActiveWeatherComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Components")
    TObjectPtr<UNiagaraComponent> LightningComponent;

    // Environment References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    TObjectPtr<ASkyLight> SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    // Weather State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    bool bWeatherActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    bool bTransitioning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float TransitionProgress = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float TransitionDuration = 5.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    float WeatherTimer = 0.0f;

    // Lightning Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningMinInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningMaxInterval = 15.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lightning")
    float NextLightningTime = 0.0f;

    // Audio Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TObjectPtr<class UAudioComponent> WeatherAudioComponent;

private:
    // Internal Functions
    void InitializeEnvironmentReferences();
    void UpdateWeatherTransition(float DeltaTime);
    void ApplyWeatherSettings(const FVFX_WeatherSettings& Settings, float Alpha = 1.0f);
    void UpdateLighting(const FVFX_WeatherSettings& Settings, float Alpha = 1.0f);
    void UpdateFog(const FVFX_WeatherSettings& Settings, float Alpha = 1.0f);
    void UpdateWeatherEffects();
    void HandleLightning(float DeltaTime);
    UNiagaraSystem* GetWeatherSystem(EVFX_WeatherType WeatherType);
    void SpawnWeatherEffect(EVFX_WeatherType WeatherType, float Intensity);
    void StopWeatherEffect();
};