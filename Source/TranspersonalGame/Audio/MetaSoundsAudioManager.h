#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "SharedTypes.h"
#include "MetaSoundsAudioManager.generated.h"

// Audio system enums for prehistoric survival game
UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Forest      UMETA(DisplayName = "Forest"), 
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Exploration UMETA(DisplayName = "Exploration"),
    Combat      UMETA(DisplayName = "Combat"),
    Stealth     UMETA(DisplayName = "Stealth"),
    Discovery   UMETA(DisplayName = "Discovery"),
    Survival    UMETA(DisplayName = "Survival")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

// Audio zone configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 1.5f;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EAudio_BiomeType::Savana;
        AmbientVolume = 0.7f;
        MusicVolume = 0.5f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

// Dynamic music system configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DynamicMusicConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    EAudio_MusicState MusicState = EAudio_MusicState::Exploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    EAudio_IntensityLevel IntensityLevel = EAudio_IntensityLevel::Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    float CrossfadeTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    bool bLoopTrack = true;

    FAudio_DynamicMusicConfig()
    {
        MusicState = EAudio_MusicState::Exploration;
        IntensityLevel = EAudio_IntensityLevel::Low;
        CrossfadeTime = 3.0f;
        bLoopTrack = true;
    }
};

/**
 * MetaSounds-powered audio manager for prehistoric survival game
 * Handles biome-based ambient audio, dynamic music system, and spatial audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UMetaSoundsAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMetaSoundsAudioManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToBiome(EAudio_BiomeType TargetBiome, float TransitionTime = 2.0f);

    // Dynamic music system
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetMusicState(EAudio_MusicState NewState, EAudio_IntensityLevel Intensity = EAudio_IntensityLevel::Medium);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void CrossfadeToTrack(USoundCue* NewTrack, float CrossfadeTime = 3.0f);

    // Spatial audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlaySpatialSound(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlayDinosaurRoar(FVector Location, float IntensityMultiplier = 1.0f);

    // Narrative audio integration
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(USoundWave* VoiceClip, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarrativeAudio();

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetSFXVolume(float Volume);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_IntensityLevel CurrentIntensity;

    // Audio components
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    UAudioComponent* NarrativeAudioComponent;

    // Biome configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioConfig> BiomeConfigs;

    // Dynamic music configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Config")
    TMap<EAudio_MusicState, FAudio_DynamicMusicConfig> MusicConfigs;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 0.8f;

private:
    // Internal audio management
    void InitializeBiomeConfigs();
    void InitializeMusicConfigs();
    void UpdateAudioComponents();
    void HandleBiomeTransition(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float TransitionTime);

    // Transition state
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 0.0f;
    EAudio_BiomeType TransitionTargetBiome;
};