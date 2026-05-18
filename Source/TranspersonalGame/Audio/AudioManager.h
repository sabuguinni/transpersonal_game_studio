#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Forest      UMETA(DisplayName = "Forest"), 
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeOutTime = 1.5f;

    FAudio_BiomeAudioSettings()
    {
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float FootstepVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float RoarVolume = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float MaxHearingDistance = 5000.0f;

    FAudio_DinosaurSoundProfile()
    {
        FootstepVolume = 1.0f;
        RoarVolume = 1.2f;
        MaxHearingDistance = 5000.0f;
    }
};

/**
 * Audio Manager - Handles biome-adaptive soundscapes, dinosaur audio, and dynamic music system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurFootstep(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void TriggerEnvironmentalSound(const FString& SoundName, const FVector& Location, float Volume = 1.0f);

    // Narrative audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarrativeClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopNarrativeClip();

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopAdaptiveMusic();

    // Audio settings
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEffectsVolume(float Volume);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    // Biome audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioSettings> BiomeAudioSettings;

    // Dinosaur sound profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    TMap<FString, FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    // Audio components
    UPROPERTY()
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    class UAudioComponent* NarrativeAudioComponent;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    float EffectsVolume = 1.0f;

private:
    void InitializeBiomeSettings();
    void InitializeDinosaurProfiles();
    void TransitionBiomeAudio(EAudio_BiomeType NewBiome);
    void UpdateMusicIntensity();
    
    FTimerHandle MusicUpdateTimer;
    bool bAdaptiveMusicActive = false;
};