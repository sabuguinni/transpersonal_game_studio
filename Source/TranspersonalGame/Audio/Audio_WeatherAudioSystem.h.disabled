#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_WeatherAudioSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Weather"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    WindStorm       UMETA(DisplayName = "Wind Storm"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_WeatherAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> IntensitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> TransitionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float BaseVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeOutTime = 2.0f;

    FAudio_WeatherAudioConfig()
    {
        BaseVolume = 0.5f;
        IntensityMultiplier = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_WeatherAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_WeatherAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherType(EAudio_WeatherType NewWeatherType, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StartWeatherTransition(EAudio_WeatherType FromWeather, EAudio_WeatherType ToWeather, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void EmergencyWeatherAlert(EAudio_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StopAllWeatherAudio();

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void UpdateWindDirection(FVector WindDirection);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetDistanceFromShelter(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void TriggerLightningStrike(FVector StrikeLocation);

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PrimaryWeatherAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* SecondaryWeatherAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* TransitionAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* EmergencyAlertAudio;

    // Weather Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    TMap<EAudio_WeatherType, FAudio_WeatherAudioConfig> WeatherConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    float GlobalWeatherVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
    float MaxAudibleDistance = 5000.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_WeatherType CurrentWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float TransitionProgress;

    // Emergency Alert System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Alerts")
    TSoftObjectPtr<USoundCue> ThunderstormAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Alerts")
    TSoftObjectPtr<USoundCue> WindStormAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Alerts")
    TSoftObjectPtr<USoundCue> SandstormAlert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Alerts")
    float AlertVolume = 0.9f;

    // Environmental Factors
    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    FVector CurrentWindDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    float DistanceFromShelter;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental")
    int32 RecentLightningStrikes;

    // Timer Handles
    FTimerHandle WeatherTransitionTimer;
    FTimerHandle IntensityUpdateTimer;
    FTimerHandle LightningResetTimer;

private:
    // Internal Functions
    void InitializeWeatherConfigs();
    void UpdateWeatherAudio();
    void HandleWeatherTransition();
    void ProcessLightningAudio(FVector StrikeLocation);
    void UpdateEnvironmentalFactors();
    void CalculateWeatherVolume();

    // Transition State
    EAudio_WeatherType TransitionFromWeather;
    EAudio_WeatherType TransitionToWeather;
    float TransitionDuration;
    float TransitionTimer;
};