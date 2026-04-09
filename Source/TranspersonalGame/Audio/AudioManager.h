#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Tense,
    Fear,
    Wonder,
    Melancholy,
    Triumph,
    Mystery,
    Danger
};

UENUM(BlueprintType)
enum class EEnvironmentalZone : uint8
{
    Forest,
    Plains,
    Mountains,
    Caves,
    Water,
    Ruins,
    DinosaurTerritory
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night,
    DeepNight
};

USTRUCT(BlueprintType)
struct FAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentalZone Zone = EEnvironmentalZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TensionLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DinosaurProximity = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0.0 to 1.0
};

/**
 * Central audio management system that handles adaptive music, environmental audio,
 * and emotional state-driven sound design for the prehistoric survival experience.
 * 
 * This system is designed around the principle that audio is the emotional conductor
 * of the player's experience - when done right, players don't notice the audio,
 * they feel the world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Core Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void InitializeAudioManager(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateAudioState(const FAudioState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEnvironmentalZone(EEnvironmentalZone NewZone, float TransitionTime = 3.0f);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void PlayAdaptiveMusic(EEmotionalState State, EEnvironmentalZone Zone);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionMusicLayers(float IntensityLevel, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopAllMusic(float FadeTime = 3.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAmbientSounds(EEnvironmentalZone Zone, ETimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalStinger(const FString& EventName, FVector Location);

    // Dinosaur Audio System
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurCall(const FString& DinosaurType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateDinosaurProximityAudio(float ProximityLevel);

    // UI and Interaction Audio
    UFUNCTION(BlueprintCallable, Category = "UI Audio")
    void PlayUISound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Interaction Audio")
    void PlayInteractionSound(const FString& InteractionType, FVector Location);

    // Voice and Dialogue
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayVoiceLine(const FString& CharacterName, const FString& LineID, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayNarration(const FString& NarrationID);

    // Audio Analysis and Feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Analysis")
    float GetCurrentMusicIntensity() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Analysis")
    TArray<float> GetSpectrumData() const;

protected:
    // Core Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* StingerComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* VoiceComponent;

    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioState CurrentAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioState TargetAudioState;

    // Audio Asset References
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music Assets")
    TMap<EEmotionalState, class UMetaSoundSource*> EmotionalMusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient Assets")
    TMap<EEnvironmentalZone, class USoundCue*> EnvironmentalAmbients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur Assets")
    TMap<FString, class USoundCue*> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Assets")
    TMap<FString, class USoundWave*> UISounds;

    // Audio Parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float EnvironmentalVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float VoiceVolume = 1.0f;

    // Internal Methods
    void UpdateMusicLayers();
    void UpdateAmbientLayers();
    void ProcessAudioTransitions(float DeltaTime);
    void CalculateEmotionalBlending();

    // Transition Management
    FTimerHandle AudioUpdateTimer;
    float TransitionProgress = 0.0f;
    bool bIsTransitioning = false;

    // World Reference
    UPROPERTY()
    UWorld* WorldContext;
};