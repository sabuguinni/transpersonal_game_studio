#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "MetasoundSource.h"
#include "GameFramework/GameModeBase.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,
    Tense,
    Danger,
    Wonder,
    Fear,
    Melancholy,
    Discovery,
    Survival
};

UENUM(BlueprintType)
enum class EEnvironmentalZone : uint8
{
    Forest = 0,
    Plains,
    Swamp,
    Mountains,
    Cave,
    River,
    Clearing,
    DenseJungle
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night,
    DeepNight
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear = 0,
    Cloudy,
    LightRain,
    HeavyRain,
    Storm,
    Fog,
    Wind
};

USTRUCT(BlueprintType)
struct FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UMetaSoundSource* MetaSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 1;
};

USTRUCT(BlueprintType)
struct FEmotionalAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState CurrentState = EEmotionalState::Calm;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 0.5f; // 0.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void Initialize();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateAudioState(float DeltaTime);

    // Emotional State Management
    UFUNCTION(BlueprintCallable, Category = "Emotional Audio")
    void SetEmotionalState(EEmotionalState NewState, float Intensity = 0.5f, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Audio")
    void BlendEmotionalStates(EEmotionalState StateA, EEmotionalState StateB, float BlendFactor);

    UFUNCTION(BlueprintPure, Category = "Emotional Audio")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState.CurrentState; }

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetEnvironmentalZone(EEnvironmentalZone Zone);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTimeOfDay(ETimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeather(EWeatherState Weather, float Intensity = 1.0f);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void PlayMusicLayer(const FString& LayerName, bool bFadeIn = true);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StopMusicLayer(const FString& LayerName, bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetMusicParameter(const FString& ParameterName, float Value);

    // Dinosaur Audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurSound(class ADinosaurBase* Dinosaur, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateDinosaurProximity(TArray<class ADinosaurBase*> NearbyDinosaurs);

    // Player Audio
    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void PlayPlayerAction(const FString& ActionType);

    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void UpdatePlayerStress(float StressLevel); // 0.0 to 1.0

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void Play3DSound(class USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateListenerPosition(FVector Position, FRotator Rotation);

protected:
    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TMap<FString, class UAudioComponent*> AudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterMusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceComponent;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EEmotionalState, FAudioLayerConfig> EmotionalMusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EEnvironmentalZone, class UMetaSoundSource*> EnvironmentalSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<EWeatherState, class UMetaSoundSource*> WeatherSounds;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FEmotionalAudioState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FEmotionalAudioState TargetEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EEnvironmentalZone CurrentZone;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    ETimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWeatherState CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float WeatherIntensity;

    // Audio Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.8f;

private:
    // Internal Methods
    void UpdateEmotionalTransition(float DeltaTime);
    void UpdateMusicLayers();
    void UpdateEnvironmentalAudio();
    void CalculateAudioOcclusion(FVector SourceLocation, FVector ListenerLocation);
    
    // Timing
    float EmotionalTransitionTimer;
    float LastAudioUpdate;
};