#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    None        UMETA(DisplayName = "None"),
    Ambient     UMETA(DisplayName = "Ambient"),
    Music       UMETA(DisplayName = "Music"),
    SFX         UMETA(DisplayName = "Sound Effects"),
    Voice       UMETA(DisplayName = "Voice/Narration"),
    Dinosaur    UMETA(DisplayName = "Dinosaur Sounds"),
    Environment UMETA(DisplayName = "Environment"),
    UI          UMETA(DisplayName = "User Interface")
};

UENUM(BlueprintType)
enum class EAudio_Priority : uint8
{
    Low         UMETA(DisplayName = "Low Priority"),
    Normal      UMETA(DisplayName = "Normal Priority"),
    High        UMETA(DisplayName = "High Priority"),
    Critical    UMETA(DisplayName = "Critical Priority")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    EAudio_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool b3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float MaxDistance;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::None;
        Priority = EAudio_Priority::Normal;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
        b3D = false;
        MaxDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PlayingSound
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Sound")
    FString SoundName;

    UPROPERTY(BlueprintReadOnly, Category = "Sound")
    UAudioComponent* AudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Sound")
    EAudio_SoundCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Sound")
    float StartTime;

    FAudio_PlayingSound()
    {
        SoundName = TEXT("");
        AudioComponent = nullptr;
        Category = EAudio_SoundCategory::None;
        StartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Sound management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FString& SoundName, USoundBase* SoundAsset, EAudio_SoundCategory Category, EAudio_Priority Priority = EAudio_Priority::Normal);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound2D(const FString& SoundName, float Volume = 1.0f, float Pitch = 1.0f, bool bLoop = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound3D(const FString& SoundName, FVector Location, float Volume = 1.0f, float Pitch = 1.0f, bool bLoop = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSoundsByCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetMasterVolume() const { return MasterVolume; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Dinosaur-specific audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayNarrationLine(const FString& NarrationKey, float Delay = 0.0f);

    // Ambient audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAmbientAudio(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeInAmbient(const FString& BiomeName, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeOutAmbient(float FadeTime = 2.0f);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TArray<FAudio_PlayingSound> PlayingSounds;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float MasterVolume;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* CurrentAmbientAudio;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    FString CurrentBiome;

private:
    void CleanupFinishedSounds();
    UAudioComponent* CreateAudioComponent(USoundBase* Sound, bool b3D = false);
    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();

    UFUNCTION()
    void OnAudioFinished();
};

#include "Audio_SoundManager.generated.h"