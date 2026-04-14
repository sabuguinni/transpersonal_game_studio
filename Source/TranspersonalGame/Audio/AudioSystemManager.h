#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    None                UMETA(DisplayName = "None"),
    Ambient            UMETA(DisplayName = "Ambient"),
    Music              UMETA(DisplayName = "Music"),
    SFX                UMETA(DisplayName = "Sound Effects"),
    Voice              UMETA(DisplayName = "Voice/Dialogue"),
    UI                 UMETA(DisplayName = "User Interface"),
    Consciousness      UMETA(DisplayName = "Consciousness Events")
};

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest             UMETA(DisplayName = "Forest"),
    Cave               UMETA(DisplayName = "Cave"),
    River              UMETA(DisplayName = "River"),
    Mountain           UMETA(DisplayName = "Mountain"),
    Sacred             UMETA(DisplayName = "Sacred Site"),
    Mystical           UMETA(DisplayName = "Mystical Realm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::None;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Environment")
    EAudio_EnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Environment")
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Environment")
    FAudio_SoundEntry BackgroundMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Environment")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Environment")
    float ReverbIntensity;

    FAudio_EnvironmentSettings()
    {
        EnvironmentType = EAudio_EnvironmentType::Forest;
        MasterVolume = 1.0f;
        ReverbIntensity = 0.5f;
    }
};

/**
 * Audio System Manager - Handles all audio playback, adaptive music, and environmental audio
 * Manages consciousness-based audio transitions and mystical soundscapes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound(const FString& SoundID, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundID, float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAmbientSound(const FString& SoundID, bool bLooping = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllAmbientSounds(float FadeOutTime = 2.0f);

    // Environment Audio
    UFUNCTION(BlueprintCallable, Category = "Audio Environment")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Environment")
    void UpdateEnvironmentSettings(const FAudio_EnvironmentSettings& NewSettings);

    // Consciousness Audio Events
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlayConsciousnessEvent(EConsciousnessLevel Level, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TriggerMysticalTransition(float Duration = 5.0f);

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void PlayBackgroundMusic(const FString& MusicID, bool bCrossfade = true, float CrossfadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StopBackgroundMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetMusicVolume(float Volume, float FadeTime = 1.0f);

    // Audio Registration
    UFUNCTION(BlueprintCallable, Category = "Audio Registration")
    void RegisterSoundEntry(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio Registration")
    void LoadAudioDatabase();

    // Volume Controls
    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

protected:
    // Audio Storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Database")
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Environment")
    FAudio_EnvironmentSettings CurrentEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Environment")
    TMap<EAudio_EnvironmentType, FAudio_EnvironmentSettings> EnvironmentPresets;

    // Active Audio Components
    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    UAudioComponent* BackgroundMusicComponent;

    UPROPERTY()
    TArray<UAudioComponent*> AmbientAudioComponents;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volume Settings")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Internal Methods
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location);
    void CleanupFinishedAudioComponents();
    void InitializeEnvironmentPresets();
    void InitializeCategoryVolumes();
    float CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry) const;

private:
    FTimerHandle CleanupTimerHandle;
    bool bIsInitialized;
};