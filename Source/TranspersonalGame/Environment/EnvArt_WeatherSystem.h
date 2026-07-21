#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EnvArt_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Sky"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Heavy Fog"),
    Dust        UMETA(DisplayName = "Dust Storm")
};

USTRUCT(BlueprintType)
struct FEnvArt_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 0.5f;

    FEnvArt_WeatherSettings()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor::White;
        SkyLightIntensity = 1.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        WindStrength = 1.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_WeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* RainParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* DustParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudio;

    // Weather State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EEnvArt_WeatherType TargetWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherChangeInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAutoWeatherChange;

    // Weather Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    TMap<EEnvArt_WeatherType, FEnvArt_WeatherSettings> WeatherPresets;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    FEnvArt_WeatherSettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    FEnvArt_WeatherSettings TargetSettings;

    // Time System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    bool bAutoTimeProgression;

    // Fog References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    class AExponentialHeightFog* HeightFog;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* RainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* WindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ThunderSound;

    // Particle Systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* RainParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* DustParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    class UParticleSystem* SnowParticleSystem;

private:
    float WeatherTransitionProgress;
    float LastWeatherChangeTime;
    FTimerHandle WeatherChangeTimer;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetWeather(EEnvArt_WeatherType NewWeather, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TriggerRandomWeatherChange();

    UFUNCTION(BlueprintPure, Category = "Weather")
    float GetTemperature() const { return CurrentSettings.Temperature; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    float GetHumidity() const { return CurrentSettings.Humidity; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    float GetWindStrength() const { return CurrentSettings.WindStrength; }

    UFUNCTION(BlueprintPure, Category = "Weather")
    bool IsRaining() const { return CurrentWeather == EEnvArt_WeatherType::Rain || CurrentWeather == EEnvArt_WeatherType::Storm; }

protected:
    // Internal Functions
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateTimeOfDay(float DeltaTime);
    void UpdateSunPosition();
    void UpdateLighting();
    void UpdateFog();
    void UpdateParticles();
    void UpdateAudio();
    void ApplyWeatherSettings(const FEnvArt_WeatherSettings& Settings);
    void InitializeWeatherPresets();
    void OnWeatherChangeTimer();
    FEnvArt_WeatherSettings LerpWeatherSettings(const FEnvArt_WeatherSettings& A, const FEnvArt_WeatherSettings& B, float Alpha);
    EEnvArt_WeatherType GetRandomWeatherType();
};