#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
    Footsteps,
    DinosaurSounds,
    WeatherSounds
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> SoundAsset;

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
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Sound playback functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(const FString& SoundName, AActor* WorldContext = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(const FString& SoundName, FVector Location, AActor* WorldContext = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSoundsByCategory(EAudio_SoundCategory Category);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetMasterVolume() const { return MasterVolume; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Sound management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundRegistered(const FString& SoundName) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundName) const;

    // Ambient and music control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientSound(const FString& SoundName, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientSound(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayMusic(const FString& MusicName, float FadeInTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopMusic(float FadeOutTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CrossfadeMusic(const FString& NewMusicName, float CrossfadeTime = 5.0f);

protected:
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    float MasterVolume;

    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    UPROPERTY()
    UAudioComponent* CurrentMusicComponent;

    UPROPERTY()
    FString CurrentAmbientSound;

    UPROPERTY()
    FString CurrentMusicTrack;

private:
    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();
    UAudioComponent* CreateAudioComponent(AActor* WorldContext = nullptr);
    float CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry) const;
};