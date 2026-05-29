#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Audio_WeatherSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Weather"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Wind        UMETA(DisplayName = "Strong Wind"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_WeatherSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TSoftObjectPtr<USoundCue> WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bLooping = true;

    FAudio_WeatherSoundData()
    {
        BaseVolume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bLooping = true;
    }
};

/**
 * Dynamic weather audio system that responds to environmental conditions
 * Manages atmospheric sounds, storm effects, and weather transitions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_WeatherSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_WeatherSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Weather sound configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_WeatherType, FAudio_WeatherSoundData> WeatherSounds;

    // Audio components for layered weather effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> PrimaryWeatherAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> SecondaryWeatherAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> ThunderAudio;

    // Current weather state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State", meta = (AllowPrivateAccess = "true"))
    EAudio_WeatherType CurrentWeather;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State", meta = (AllowPrivateAccess = "true"))
    EAudio_WeatherType TargetWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control", meta = (AllowPrivateAccess = "true"))
    float WeatherTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Control", meta = (AllowPrivateAccess = "true"))
    float WeatherIntensity = 1.0f;

    // Thunder system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thunder", meta = (AllowPrivateAccess = "true"))
    TArray<TSoftObjectPtr<USoundCue>> ThunderSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thunder", meta = (AllowPrivateAccess = "true"))
    float ThunderMinInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thunder", meta = (AllowPrivateAccess = "true"))
    float ThunderMaxInterval = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thunder", meta = (AllowPrivateAccess = "true"))
    float ThunderVolume = 0.8f;

    // Internal state
    float CurrentTransitionTime;
    float ThunderTimer;
    float NextThunderTime;
    bool bIsTransitioning;

public:
    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherType(EAudio_WeatherType NewWeatherType, bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    void SetWeatherIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    EAudio_WeatherType GetCurrentWeatherType() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Weather Audio")
    float GetCurrentIntensity() const { return WeatherIntensity; }

    // Thunder control
    UFUNCTION(BlueprintCallable, Category = "Thunder")
    void TriggerThunder(bool bRandomDelay = false);

    UFUNCTION(BlueprintCallable, Category = "Thunder")
    void SetThunderEnabled(bool bEnabled);

private:
    void InitializeAudioComponents();
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateThunderSystem(float DeltaTime);
    void PlayWeatherSound(EAudio_WeatherType WeatherType, UAudioComponent* AudioComponent);
    void StopWeatherSound(UAudioComponent* AudioComponent, float FadeTime = 2.0f);
    float CalculateVolumeForWeather(EAudio_WeatherType WeatherType) const;
    void ScheduleNextThunder();
};