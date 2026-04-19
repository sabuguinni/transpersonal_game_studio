#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "AudioSystemManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SYSTEM MANAGER
 * Audio Agent #16
 * 
 * Central audio management system for the prehistoric survival game.
 * Handles ambient soundscapes, dynamic music, 3D positional audio,
 * and adaptive audio based on gameplay state and environment.
 */

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient = 0         UMETA(DisplayName = "Ambient"),
    Music = 1           UMETA(DisplayName = "Music"),
    SFX = 2             UMETA(DisplayName = "SFX"),
    Voice = 3           UMETA(DisplayName = "Voice"),
    UI = 4              UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EAudio_Environment : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Plains = 1          UMETA(DisplayName = "Plains"),
    Swamp = 2           UMETA(DisplayName = "Swamp"),
    Mountains = 3       UMETA(DisplayName = "Mountains"),
    Cave = 4            UMETA(DisplayName = "Cave"),
    River = 5           UMETA(DisplayName = "River")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Caution = 1         UMETA(DisplayName = "Caution"),
    Danger = 2          UMETA(DisplayName = "Danger"),
    Combat = 3          UMETA(DisplayName = "Combat"),
    Panic = 4           UMETA(DisplayName = "Panic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundType SoundType = EAudio_SoundType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool b3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationDistance = 1000.0f;

    FAudio_SoundEntry()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        b3D = true;
        AttenuationDistance = 1000.0f;
        SoundType = EAudio_SoundType::Ambient;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAudio_Environment Environment = EAudio_Environment::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FAudio_SoundEntry BackgroundMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float ReverbAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float EchoDelay = 0.1f;

    FAudio_EnvironmentSettings()
    {
        Environment = EAudio_Environment::Forest;
        ReverbAmount = 0.3f;
        EchoDelay = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound(const FAudio_SoundEntry& SoundEntry, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSoundsByType(EAudio_SoundType SoundType);

    // Environment Audio
    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void SetEnvironment(EAudio_Environment NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void UpdateAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void PlayBackgroundMusic(USoundCue* MusicCue, bool bFadeIn = true);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StopBackgroundMusic(bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetMusicIntensity(float Intensity);

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetVoiceVolume(float Volume);

    // Audio Analysis
    UFUNCTION(BlueprintCallable, Category = "Audio Analysis")
    float GetCurrentAudioLevel();

    UFUNCTION(BlueprintCallable, Category = "Audio Analysis")
    bool IsLocationAudible(FVector Location, float MaxDistance = 1000.0f);

protected:
    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* BackgroundMusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_Environment CurrentEnvironment = EAudio_Environment::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentMusicIntensity = 0.5f;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float AmbientVolume = 0.6f;

    // Environment Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Settings")
    TMap<EAudio_Environment, FAudio_EnvironmentSettings> EnvironmentSettings;

    // Internal Methods
    void InitializeEnvironmentSettings();
    void CleanupInactiveComponents();
    UAudioComponent* CreateAudioComponent();
    void ApplyVolumeSettings(UAudioComponent* AudioComponent, EAudio_SoundType SoundType);

private:
    FTimerHandle CleanupTimer;
    FTimerHandle AmbientUpdateTimer;
};