#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

// Audio-specific enums
UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Master = 0          UMETA(DisplayName = "Master"),
    Music = 1           UMETA(DisplayName = "Music"),
    SFX = 2             UMETA(DisplayName = "SFX"),
    Ambience = 3        UMETA(DisplayName = "Ambience"),
    Voice = 4           UMETA(DisplayName = "Voice"),
    UI = 5              UMETA(DisplayName = "UI"),
    Dinosaur = 6        UMETA(DisplayName = "Dinosaur")
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
    Coast = 6           UMETA(DisplayName = "Coast")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn = 0            UMETA(DisplayName = "Dawn"),
    Day = 1             UMETA(DisplayName = "Day"),
    Dusk = 2            UMETA(DisplayName = "Dusk"),
    Night = 3           UMETA(DisplayName = "Night")
};

// Audio configuration structs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_VolumeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbienceVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float UIVolume = 0.8f;

    FAudio_VolumeSettings()
    {
        MasterVolume = 1.0f;
        MusicVolume = 0.8f;
        SFXVolume = 1.0f;
        AmbienceVolume = 0.6f;
        VoiceVolume = 1.0f;
        UIVolume = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> DayAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> NightAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime = 2.0f;

    FAudio_BiomeConfig()
    {
        BiomeType = EAudio_BiomeAmbience::Forest;
        BaseVolume = 0.6f;
        FadeTime = 2.0f;
    }
};

/**
 * Audio System Manager - Handles all audio functionality for the prehistoric survival game
 * Manages adaptive music, environmental audio, dinosaur sounds, and player feedback
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownAudioSystem();

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetVolumeSettings(const FAudio_VolumeSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FAudio_VolumeSettings GetVolumeSettings() const { return VolumeSettings; }

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Biome and environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeAmbience NewBiome, bool bFadeTransition = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay, bool bFadeTransition = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateEnvironmentalAudio(const FVector& PlayerLocation);

    // Sound playback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySoundAtLocation(USoundBase* Sound, const FVector& Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySound2D(USoundBase* Sound, EAudio_SoundCategory Category = EAudio_SoundCategory::SFX, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSoundsInCategory(EAudio_SoundCategory Category);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurActor(AActor* DinosaurActor, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterDinosaurActor(AActor* DinosaurActor);

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayMusicTrack(USoundBase* MusicTrack, bool bLoop = true, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity); // 0.0 = calm, 1.0 = combat

    // Audio events and triggers
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerAudioEvent(const FString& EventName, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterAudioTrigger(const FString& TriggerName, USoundBase* Sound, EAudio_SoundCategory Category);

    // Performance and optimization
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioQuality(EEng_PerformanceTier QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void CullDistantSounds(const FVector& ListenerLocation, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    int32 GetActiveAudioComponentCount() const;

protected:
    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_VolumeSettings VolumeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    int32 MaxConcurrentSounds = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AudioCullInterval = 1.0f;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeAmbience CurrentBiome = EAudio_BiomeAmbience::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentMusicIntensity = 0.0f;

    // Audio components
    UPROPERTY()
    UAudioComponent* AmbienceComponent;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    // Registered actors and sounds
    UPROPERTY()
    TMap<FString, TSoftObjectPtr<USoundBase>> AudioEventMap;

    UPROPERTY()
    TMap<AActor*, FString> RegisteredDinosaurs;

    // Timers
    FTimerHandle AudioCullTimer;

private:
    // Internal methods
    void UpdateAmbienceForBiome();
    void UpdateMusicForIntensity();
    void CleanupFinishedAudioComponents();
    FAudio_BiomeConfig* GetBiomeConfig(EAudio_BiomeAmbience BiomeType);
    float CalculateVolumeForCategory(EAudio_SoundCategory Category) const;
};