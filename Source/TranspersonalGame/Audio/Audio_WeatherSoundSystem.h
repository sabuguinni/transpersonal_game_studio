#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Audio_WeatherSoundSystem.generated.h"

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_WeatherSoundSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_WeatherSoundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Sound Management
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherType(EWeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StartWeatherTransition(EWeatherType TargetWeather, float TransitionDuration);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void StopAllWeatherSounds();

protected:
    // Weather Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> RainAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> ThunderAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> FogAudioComponent;

    // Weather Sound Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> RainLightSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> RainHeavySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> ThunderDistantSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> ThunderCloseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> WindGentleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> WindStrongSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Sounds")
    TObjectPtr<USoundCue> FogAmbienceSound;

    // Current Weather State
    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    EWeatherType CurrentWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float CurrentWeatherIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning;

    // Transition Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Transition")
    float TransitionDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    float TransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    EWeatherType TargetWeatherType;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    EWeatherType StartWeatherType;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float ThunderRandomInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Settings")
    float ThunderTimer;

private:
    void UpdateWeatherAudio(float DeltaTime);
    void ProcessWeatherTransition(float DeltaTime);
    void PlayThunderSound();
    void UpdateAudioComponentVolumes();
    float CalculateWeatherBlend(EWeatherType WeatherA, EWeatherType WeatherB, float BlendFactor);
};