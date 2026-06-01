#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Music,
    SFX,
    Voice,
    UI,
    Dinosaur,
    Environment,
    Combat
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest,
    Savanna,
    Swamp,
    Desert,
    Mountain
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime = 0.0f;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrossfadeTime = 2.0f;

    FAudio_BiomeAmbience()
    {
        BiomeType = EAudio_BiomeType::Forest;
        CrossfadeTime = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(const FString& SoundID, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiomeAmbience(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayCraftingSound(const FString& MaterialType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FString& SurfaceType, FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAmbience> BiomeAmbiences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* CurrentAmbienceComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    UAudioComponent* CurrentMusicComponent;

private:
    void InitializeDefaultSounds();
    void InitializeBiomeAmbiences();
    void InitializeCategoryVolumes();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location);
    void CleanupFinishedAudioComponents();
};