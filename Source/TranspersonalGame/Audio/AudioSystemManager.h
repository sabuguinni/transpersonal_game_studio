#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioCategory : uint8
{
    Ambient,
    Music,
    SFX,
    Voice,
    UI,
    Dinosaur
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana,
    Forest,
    Desert,
    Swamp,
    Mountain
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_AudioCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_AudioCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.5f;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EAudio_BiomeType::Savana;
        AmbientVolume = 0.7f;
        MusicVolume = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FString& SoundName, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiomeAmbient(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllAmbient();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_AudioCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsPlayingAmbient() const { return CurrentAmbientComponent && CurrentAmbientComponent->IsPlaying(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<EAudio_AudioCategory, float> CategoryVolumes;

    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    UPROPERTY()
    UAudioComponent* CurrentMusicComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float MasterVolume = 1.0f;

private:
    void InitializeDefaultSounds();
    void InitializeBiomeConfigs();
    UAudioComponent* CreateAudioComponent();
    FAudio_SoundEntry* FindSound(const FString& SoundName);
};