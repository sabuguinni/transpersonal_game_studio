#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "../Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

// Audio-specific enums
UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Music = 0           UMETA(DisplayName = "Music"),
    SFX = 1             UMETA(DisplayName = "Sound Effects"),
    Ambience = 2        UMETA(DisplayName = "Ambience"),
    Voice = 3           UMETA(DisplayName = "Voice"),
    UI = 4              UMETA(DisplayName = "UI"),
    Footsteps = 5       UMETA(DisplayName = "Footsteps"),
    Combat = 6          UMETA(DisplayName = "Combat"),
    Dinosaurs = 7       UMETA(DisplayName = "Dinosaurs")
};

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration = 0     UMETA(DisplayName = "Exploration"),
    Tension = 1         UMETA(DisplayName = "Tension"),
    Combat = 2          UMETA(DisplayName = "Combat"),
    Danger = 3          UMETA(DisplayName = "Danger"),
    Safe = 4            UMETA(DisplayName = "Safe"),
    Night = 5           UMETA(DisplayName = "Night"),
    Day = 6             UMETA(DisplayName = "Day")
};

UENUM(BlueprintType)
enum class EAudio_Priority : uint8
{
    Critical = 0        UMETA(DisplayName = "Critical"),
    High = 1            UMETA(DisplayName = "High"),
    Medium = 2          UMETA(DisplayName = "Medium"),
    Low = 3             UMETA(DisplayName = "Low")
};

// Audio configuration struct
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category = EAudio_SoundCategory::SFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_Priority Priority = EAudio_Priority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool b3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationDistance = 1000.0f;

    FAudio_SoundConfig()
    {
        Category = EAudio_SoundCategory::SFX;
        Priority = EAudio_Priority::Medium;
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
        bLooping = false;
        b3D = true;
        AttenuationDistance = 1000.0f;
    }
};

// Music transition data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState FromState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState ToState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TransitionTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bCrossfade = true;

    FAudio_MusicTransition()
    {
        FromState = EAudio_MusicState::Exploration;
        ToState = EAudio_MusicState::Exploration;
        TransitionTime = 2.0f;
        bCrossfade = true;
    }
};

/**
 * Audio System Manager - Handles all audio in the game
 * Manages adaptive music, 3D sound effects, ambient audio, and voice
 * Uses MetaSounds for procedural audio generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound2D(USoundBase* Sound, const FAudio_SoundConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound3D(USoundBase* Sound, const FVector& Location, const FAudio_SoundConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Music")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void FadeOutMusic(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void FadeInMusic(float FadeTime = 2.0f);

    // Ambient audio
    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void SetAmbienceState(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Ambience")
    void UpdateTimeOfDay(float TimeOfDay);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    void PlayDinosaurRoar(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    void PlayDinosaurFootsteps(const FVector& Location, float Weight = 1.0f);

    // Player audio
    UFUNCTION(BlueprintCallable, Category = "Player")
    void PlayPlayerFootstep(const FVector& Location, const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Player")
    void PlayPlayerHeartbeat(float Intensity = 1.0f);

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Audio occlusion and reverb
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioOcclusion(const FVector& ListenerLocation, const FVector& SoundLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetReverbZone(const FString& ReverbPreset);

private:
    // Current state
    UPROPERTY()
    EAudio_MusicState CurrentMusicState;

    UPROPERTY()
    EEng_BiomeType CurrentBiome;

    UPROPERTY()
    float CurrentTimeOfDay;

    // Volume settings
    UPROPERTY()
    float MasterVolume;

    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Active audio components
    UPROPERTY()
    TMap<EAudio_SoundCategory, UAudioComponent*> ActiveSounds;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    UAudioComponent* AmbienceComponent;

    // Sound assets
    UPROPERTY()
    TMap<EAudio_MusicState, USoundBase*> MusicTracks;

    UPROPERTY()
    TMap<EEng_BiomeType, USoundBase*> AmbienceTracks;

    UPROPERTY()
    TArray<USoundBase*> DinosaurRoars;

    UPROPERTY()
    TArray<USoundBase*> DinosaurFootsteps;

    UPROPERTY()
    TArray<USoundBase*> PlayerFootsteps;

    // Internal functions
    void LoadAudioAssets();
    void InitializeVolumeSettings();
    void TransitionMusic(EAudio_MusicState NewState);
    UAudioComponent* CreateAudioComponent();
    void CleanupFinishedSounds();
};