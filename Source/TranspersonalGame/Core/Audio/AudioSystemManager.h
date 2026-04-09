// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioSystemManager.generated.h"

class UMetaSoundSource;
class USoundAttenuation;
class USoundConcurrency;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioStateChanged, FString, StateName, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicTransition, FString, NewMusicState);

/**
 * Enum for different audio environment types in the prehistoric world
 */
UENUM(BlueprintType)
enum class EAudioEnvironment : uint8
{
    Forest_Dense        UMETA(DisplayName = "Dense Forest"),
    Forest_Sparse       UMETA(DisplayName = "Sparse Forest"),
    Plains_Open         UMETA(DisplayName = "Open Plains"),
    River_Flowing       UMETA(DisplayName = "Flowing River"),
    Cave_Deep          UMETA(DisplayName = "Deep Cave"),
    Cave_Shallow       UMETA(DisplayName = "Shallow Cave"),
    Swamp_Murky        UMETA(DisplayName = "Murky Swamp"),
    Mountain_Peak      UMETA(DisplayName = "Mountain Peak"),
    Valley_Sheltered   UMETA(DisplayName = "Sheltered Valley")
};

/**
 * Enum for emotional states that drive adaptive music
 */
UENUM(BlueprintType)
enum class EMusicEmotionalState : uint8
{
    Calm_Exploration    UMETA(DisplayName = "Calm Exploration"),
    Tense_Awareness     UMETA(DisplayName = "Tense Awareness"),
    Fear_Immediate      UMETA(DisplayName = "Immediate Fear"),
    Wonder_Discovery    UMETA(DisplayName = "Wonder and Discovery"),
    Melancholy_Loss     UMETA(DisplayName = "Melancholy and Loss"),
    Hope_Progress       UMETA(DisplayName = "Hope and Progress"),
    Silence_Suspense    UMETA(DisplayName = "Suspenseful Silence")
};

/**
 * Struct for audio layer configuration in adaptive music
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    UMetaSoundSource* MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    int32 Priority = 0;

    FAudioLayerConfig()
    {
        MetaSoundAsset = nullptr;
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bLooping = true;
        Priority = 0;
    }
};

/**
 * Struct for dinosaur audio profile - each species has unique audio characteristics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<USoundCue*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<USoundCue*> MovementSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<USoundCue*> AlertSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<USoundCue*> AggressiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    USoundAttenuation* AttenuationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeVariance = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float PitchVariance = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float CallFrequency = 30.0f; // Seconds between calls

    FDinosaurAudioProfile()
    {
        AttenuationSettings = nullptr;
        VolumeVariance = 0.2f;
        PitchVariance = 0.15f;
        CallFrequency = 30.0f;
    }
};

/**
 * Audio System Manager - Handles all audio in the prehistoric world
 * Implements adaptive music, environmental audio, and dinosaur soundscapes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioStateChanged OnAudioStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnMusicTransition OnMusicTransition;

    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioEnvironment(EAudioEnvironment NewEnvironment, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicEmotionalState(EMusicEmotionalState NewState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurSpecies, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurAudioProfile(const FString& SpeciesName, const FDinosaurAudioProfile& Profile);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWindIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateTimeOfDay(float TimeNormalized); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TriggerWeatherTransition(const FString& WeatherType, float Intensity);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void AddMusicLayer(const FString& LayerName, const FAudioLayerConfig& LayerConfig);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicLayerVolume(const FString& LayerName, float Volume, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void EnableMusicLayer(const FString& LayerName, bool bEnable, float FadeTime = 2.0f);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void Play3DSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    UAudioComponent* PlayLooping3DSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio System")
    EAudioEnvironment GetCurrentEnvironment() const { return CurrentEnvironment; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    EMusicEmotionalState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    float GetMasterVolume() const { return MasterVolume; }

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioEnvironment CurrentEnvironment;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EMusicEmotionalState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolume;

    // Audio layer management
    UPROPERTY()
    TMap<FString, FAudioLayerConfig> MusicLayers;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    // Dinosaur audio profiles
    UPROPERTY()
    TMap<FString, FDinosaurAudioProfile> DinosaurProfiles;

    // Environmental audio components
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* WindAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

private:
    void InitializeAudioComponents();
    void UpdateEnvironmentalAudio();
    void TransitionMusicLayers(EMusicEmotionalState NewState, float TransitionTime);
    void CleanupInactiveComponents();

    // Timers
    FTimerHandle EnvironmentUpdateTimer;
    FTimerHandle ComponentCleanupTimer;
};