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

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SYSTEM MANAGER
 * Audio Agent #16
 * 
 * Manages all audio systems: ambient soundscapes, dynamic music, 
 * dinosaur audio, environmental effects, and spatial audio.
 * Uses UE5 MetaSounds for procedural audio generation.
 */

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient = 0         UMETA(DisplayName = "Ambient"),
    Music = 1           UMETA(DisplayName = "Music"),
    SFX = 2             UMETA(DisplayName = "SFX"),
    Voice = 3           UMETA(DisplayName = "Voice"),
    Dinosaur = 4        UMETA(DisplayName = "Dinosaur"),
    Environment = 5     UMETA(DisplayName = "Environment"),
    UI = 6              UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EAudio_BiomeAmbience : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Plains = 1          UMETA(DisplayName = "Plains"),
    Swamp = 2           UMETA(DisplayName = "Swamp"),
    Desert = 3          UMETA(DisplayName = "Desert"),
    Mountains = 4       UMETA(DisplayName = "Mountains"),
    River = 5           UMETA(DisplayName = "River"),
    Cave = 6            UMETA(DisplayName = "Cave"),
    Coast = 7           UMETA(DisplayName = "Coast")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 1000.0f;

    FAudio_SoundLayer()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bIsLooping = true;
        bIs3D = true;
        AttenuationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<UMetaSoundSource> DynamicAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FAudio_BiomeProfile()
    {
        BiomeType = EAudio_BiomeAmbience::Forest;
        BaseVolume = 0.7f;
        FadeInTime = 3.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MovementSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> DeathSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxHearingDistance = 2000.0f;

    FAudio_DinosaurProfile()
    {
        DinosaurSpecies = TEXT("Unknown");
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.2f;
        MaxHearingDistance = 2000.0f;
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

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownAudioSystem();

    // Biome Ambience System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeAmbience NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToBiome(EAudio_BiomeAmbience TargetBiome, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbienceIntensity(float Intensity);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDynamicMusic(const FString& MusicState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopDynamicMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity);

    // Dinosaur Audio System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& Species, EAudio_SoundType SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurProfile(const FAudio_DinosaurProfile& Profile);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalSound(TSoftObjectPtr<USoundBase> Sound, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetWeatherAudio(const FString& WeatherType, float Intensity);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateListenerPosition(FVector Position, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioOcclusion(bool bEnabled);

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundType Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetCategoryVolume(EAudio_SoundType Category) const;

protected:
    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UAudioComponent> AmbienceComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UAudioComponent> MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UAudioComponent> WeatherComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    EAudio_BiomeAmbience CurrentBiome = EAudio_BiomeAmbience::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    float CurrentAmbienceIntensity = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    float CurrentMusicIntensity = 0.5f;

    // Audio Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<EAudio_BiomeAmbience, FAudio_BiomeProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<FString, FAudio_DinosaurProfile> DinosaurProfiles;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<EAudio_SoundType, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVolume = 1.0f;

    // System State
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    bool bAudioOcclusionEnabled = true;

private:
    // Internal Methods
    void LoadBiomeProfiles();
    void LoadDinosaurProfiles();
    void InitializeVolumeSettings();
    void CreateAudioComponents();
    void UpdateBiomeAmbience();
    void ProcessAudioOcclusion();
    
    // Transition System
    void HandleBiomeTransition();
    bool bIsTransitioning = false;
    EAudio_BiomeAmbience TransitionTargetBiome = EAudio_BiomeAmbience::Forest;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 3.0f;
};