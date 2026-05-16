#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana       UMETA(DisplayName = "Savana"),
    Pantano      UMETA(DisplayName = "Pantano"),
    Floresta     UMETA(DisplayName = "Floresta"),
    Deserto      UMETA(DisplayName = "Deserto"),
    Montanha     UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe         UMETA(DisplayName = "Safe"),
    Cautious     UMETA(DisplayName = "Cautious"),
    Danger       UMETA(DisplayName = "Danger"),
    Combat       UMETA(DisplayName = "Combat")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime = 3.0f;

    FAudio_BiomeAudioConfig()
    {
        BaseVolume = 0.7f;
        FadeTime = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ThreatAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Priority = 1.0f;

    FAudio_ThreatAudioConfig()
    {
        Volume = 0.8f;
        Priority = 1.0f;
    }
};

/**
 * Audio Manager - Handles biome-adaptive soundscapes and dynamic audio systems
 * Manages environmental audio, threat-based music transitions, and spatial audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurFootsteps(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalSound(const FString& SoundName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float Volume);

    // Audio component management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* CreateAudioComponent(USoundBase* Sound, const FVector& Location, bool bAutoDestroy = true);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_ThreatLevel CurrentThreatLevel;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TArray<UAudioComponent*> TemporaryAudioComponents;

    // Audio configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioConfig> BiomeAudioConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_ThreatLevel, FAudio_ThreatAudioConfig> ThreatAudioConfigs;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume;

private:
    void InitializeAudioConfigs();
    void TransitionToNewBiome(EAudio_BiomeType NewBiome);
    void TransitionToNewThreatLevel(EAudio_ThreatLevel NewThreatLevel);
    void CleanupTemporaryAudioComponents();
    
    FTimerHandle CleanupTimerHandle;
};