#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "EnvArt_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sunny"),
    PartlyCloudy    UMETA(DisplayName = "Partly Cloudy"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
    Mist            UMETA(DisplayName = "Light Mist")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WeatherConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    bool bEnableRainParticles = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    float RainIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_WeatherSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather Control
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeatherType(EEnvArt_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EEnvArt_WeatherType NewWeatherType, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintPure, Category = "Weather")
    EEnvArt_WeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetTimeOfDay(float TimeInHours);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateSunPosition();

    // Manual Controls
    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor = true)
    void SetSunIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting", CallInEditor = true)
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Effects", CallInEditor = true)
    void ToggleRainEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather", CallInEditor = true)
    void ApplyPresetWeather();

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* RainParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* MistParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    // Weather Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeatherType = EEnvArt_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    TMap<EEnvArt_WeatherType, FEnvArt_WeatherConfig> WeatherConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float CurrentTimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float SunAngleOffset = 0.0f;

    // Transition System
    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather")
    float TransitionProgress = 0.0f;

    UPROPERTY()
    float TransitionDuration = 5.0f;

    UPROPERTY()
    EEnvArt_WeatherType TargetWeatherType;

    UPROPERTY()
    FEnvArt_WeatherConfig StartWeatherConfig;

    UPROPERTY()
    FEnvArt_WeatherConfig TargetWeatherConfig;

    // Asset References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UNiagaraSystem* RainParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UNiagaraSystem* MistParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class USoundBase* RainAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class USoundBase* WindAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class USoundBase* ForestAmbientSound;

private:
    void InitializeWeatherConfigs();
    void ApplyWeatherConfig(const FEnvArt_WeatherConfig& Config);
    void UpdateTransition(float DeltaTime);
    FEnvArt_WeatherConfig LerpWeatherConfigs(const FEnvArt_WeatherConfig& A, const FEnvArt_WeatherConfig& B, float Alpha);
    void UpdateAmbientAudio();
    void UpdateParticleEffects();
};