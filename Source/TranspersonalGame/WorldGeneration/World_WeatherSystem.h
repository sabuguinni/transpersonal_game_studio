#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "World_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
    Sandstorm   UMETA(DisplayName = "Sandstorm"),
    Snow        UMETA(DisplayName = "Snow")
};

UENUM(BlueprintType)
enum class EWorld_ClimateZone : uint8
{
    Tropical    UMETA(DisplayName = "Tropical"),
    Temperate   UMETA(DisplayName = "Temperate"),
    Arid        UMETA(DisplayName = "Arid"),
    Polar       UMETA(DisplayName = "Polar"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_WeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float WindSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "-50.0", ClampMax = "50.0"))
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Visibility = 1.0f;

    FWorld_WeatherData()
    {
        WeatherType = EWorld_WeatherType::Clear;
        Intensity = 0.5f;
        WindSpeed = 10.0f;
        Humidity = 50.0f;
        Temperature = 20.0f;
        Visibility = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WeatherSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* RainParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SnowParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SandstormParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WeatherAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FWorld_WeatherData CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    EWorld_ClimateZone ClimateZone = EWorld_ClimateZone::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "1.0", ClampMax = "3600.0"))
    float WeatherCycleDuration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherTransitionSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableRandomWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    bool bEnableSeasonalChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* RainSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* StormSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* WindSound;

private:
    float WeatherTimer;
    float TransitionTimer;
    FWorld_WeatherData TargetWeather;
    bool bIsTransitioning;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeather(EWorld_WeatherType NewWeatherType, float NewIntensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void StartWeatherTransition(const FWorld_WeatherData& NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FWorld_WeatherData GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetClimateZone(EWorld_ClimateZone NewClimateZone);

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateFogSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateLightingSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateAudioEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void GenerateRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    bool IsStormActive() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetVisibilityMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FVector GetWindDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Weather System")
    float GetWindStrength() const;

protected:
    void UpdateWeatherCycle(float DeltaTime);
    void ProcessWeatherTransition(float DeltaTime);
    FWorld_WeatherData GenerateWeatherForClimate() const;
    void ApplyWeatherEffects();
    void CleanupWeatherEffects();
};

#include "World_WeatherSystem.generated.h"