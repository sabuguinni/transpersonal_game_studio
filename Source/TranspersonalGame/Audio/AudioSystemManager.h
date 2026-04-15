#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "../Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

// Forward declarations
class UAudioComponent;
class USoundCue;
class USoundWave;
class UMetaSoundSource;

/**
 * Audio system enums and structs for prehistoric survival game
 * All audio types use "Audio_" prefix to avoid conflicts
 */

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient = 0         UMETA(DisplayName = "Ambient"),
    Music = 1           UMETA(DisplayName = "Music"),
    SFX = 2             UMETA(DisplayName = "Sound Effects"),
    Voice = 3           UMETA(DisplayName = "Voice/Narration"),
    UI = 4              UMETA(DisplayName = "User Interface"),
    Dinosaur = 5        UMETA(DisplayName = "Dinosaur Sounds"),
    Environment = 6     UMETA(DisplayName = "Environment"),
    Combat = 7          UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EAudio_BiomeAmbience : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Grassland = 1       UMETA(DisplayName = "Grassland"),
    Desert = 2          UMETA(DisplayName = "Desert"),
    Swamp = 3           UMETA(DisplayName = "Swamp"),
    Mountains = 4       UMETA(DisplayName = "Mountains"),
    River = 5           UMETA(DisplayName = "River"),
    Coast = 6           UMETA(DisplayName = "Coast"),
    Cave = 7            UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn = 0            UMETA(DisplayName = "Dawn"),
    Morning = 1         UMETA(DisplayName = "Morning"),
    Midday = 2          UMETA(DisplayName = "Midday"),
    Afternoon = 3       UMETA(DisplayName = "Afternoon"),
    Dusk = 4            UMETA(DisplayName = "Dusk"),
    Night = 5           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundSettings
{
    GENERATED_BODY()

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
    bool b3DSound = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationDistance = 1000.0f;

    FAudio_SoundSettings()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
        bLooping = false;
        b3DSound = true;
        AttenuationDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundCue>> AmbienceLoops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundCue>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<UMetaSoundSource>> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MusicCrossfadeTime = 3.0f;

    FAudio_BiomeAudioData()
    {
        BiomeType = EAudio_BiomeAmbience::Forest;
        RandomSoundInterval = 10.0f;
        MusicCrossfadeTime = 3.0f;
    }
};

/**
 * Main audio system manager for the prehistoric survival game
 * Handles adaptive music, environmental audio, and sound effects
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio system management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioSystem(float DeltaTime);

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(EAudio_BiomeAmbience NewBiome, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    // Sound playback
    UFUNCTION(BlueprintCallable, Category = "Audio Playback")
    UAudioComponent* PlaySound2D(USoundBase* Sound, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Playback")
    UAudioComponent* PlaySound3D(USoundBase* Sound, FVector Location, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Playback")
    void StopSound(UAudioComponent* AudioComponent, float FadeOutTime = 0.0f);

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void PlayMusic(UMetaSoundSource* MusicTrack, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StopMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void CrossfadeMusic(UMetaSoundSource* NewTrack, float CrossfadeTime = 3.0f);

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetAmbienceVolume(float Volume);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio System")
    EAudio_BiomeAmbience GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    bool IsAudioSystemInitialized() const { return bIsInitialized; }

protected:
    // Internal audio management
    void UpdateBiomeAudio(float DeltaTime);
    void UpdateRandomSounds(float DeltaTime);
    void TransitionBiomeAudio();
    void LoadBiomeAudioData();

    // Audio component management
    void CleanupFinishedAudioComponents();
    UAudioComponent* CreateAudioComponent(USoundBase* Sound);

private:
    // System state
    UPROPERTY()
    bool bIsInitialized = false;

    UPROPERTY()
    EAudio_BiomeAmbience CurrentBiome = EAudio_BiomeAmbience::Forest;

    UPROPERTY()
    EAudio_BiomeAmbience TargetBiome = EAudio_BiomeAmbience::Forest;

    UPROPERTY()
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Midday;

    UPROPERTY()
    bool bIsTransitioningBiome = false;

    UPROPERTY()
    float BiomeTransitionTime = 0.0f;

    UPROPERTY()
    float BiomeTransitionDuration = 2.0f;

    // Audio components
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    UAudioComponent* CurrentMusicComponent = nullptr;

    UPROPERTY()
    UAudioComponent* CurrentAmbienceComponent = nullptr;

    // Biome audio data
    UPROPERTY()
    TMap<EAudio_BiomeAmbience, FAudio_BiomeAudioData> BiomeAudioData;

    // Volume settings
    UPROPERTY()
    float MasterVolume = 1.0f;

    UPROPERTY()
    float MusicVolume = 0.7f;

    UPROPERTY()
    float SFXVolume = 1.0f;

    UPROPERTY()
    float AmbienceVolume = 0.8f;

    // Random sound timing
    UPROPERTY()
    float RandomSoundTimer = 0.0f;

    UPROPERTY()
    float NextRandomSoundTime = 10.0f;
};