#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "../SharedTypes.h"
#include "Audio_SystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    USoundWave* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIsLooping = true;

    FAudio_SoundLayer()
    {
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bIsLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    TArray<FAudio_SoundLayer> ActiveLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    float TensionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EBiomeType CurrentBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    ETimeOfDay TimeOfDay = ETimeOfDay::Day;

    FAudio_MusicState()
    {
        TensionLevel = 0.0f;
        CurrentBiome = EBiomeType::Forest;
        TimeOfDay = ETimeOfDay::Day;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    UAudioComponent* SFXAudioComponent;

    // Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TMap<EBiomeType, FAudio_SoundLayer> BiomeAmbienceLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TMap<ETimeOfDay, FAudio_SoundLayer> TimeOfDayLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TArray<FAudio_SoundLayer> TensionLayers;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.6f;

    // Dynamic Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTensionLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome = EBiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Day;

public:
    // Music Control Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateMusicState(EBiomeType NewBiome, ETimeOfDay NewTimeOfDay, float TensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTensionLevel(float NewTensionLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySoundEffect(USoundWave* SoundAsset, float Volume = 1.0f, bool bRandomizePitch = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(EDinosaurSpecies Species, EDinosaurBehaviorState BehaviorState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarrativeAudio(const FString& AudioPath, float Volume = 1.0f);

    // Volume Control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbienceVolume(float Volume);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StartEnvironmentalAudio(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopEnvironmentalAudio();

private:
    void UpdateAdaptiveMusic();
    void CrossfadeLayers(const TArray<FAudio_SoundLayer>& NewLayers);
    void InitializeAudioLayers();
    
    // Internal state tracking
    float LastTensionUpdate = 0.0f;
    float TensionUpdateInterval = 1.0f;
};