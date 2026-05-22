#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioType : uint8
{
    SFX,
    Music,
    Ambience,
    Voice,
    UI
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
    EAudio_AudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        AudioType = EAudio_AudioType::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
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
    void PlaySound(const FString& SoundID, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySoundAtLocation(const FString& SoundID, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioTypeVolume(EAudio_AudioType AudioType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSound(const FString& SoundID, USoundCue* SoundCue, EAudio_AudioType AudioType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void LoadAudioDatabase();

    // Prehistoric-specific audio functions
    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayDinosaurRoar(const FString& DinosaurType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayFootstepSound(const FString& SurfaceType, FVector Location, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayAmbienceForBiome(const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    void PlayWeatherAudio(const FString& WeatherType, float Intensity = 1.0f);

protected:
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> SoundDatabase;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    float MasterVolume = 1.0f;

    UPROPERTY()
    TMap<EAudio_AudioType, float> AudioTypeVolumes;

    UPROPERTY()
    UAudioComponent* CurrentAmbienceComponent;

    UPROPERTY()
    UAudioComponent* CurrentMusicComponent;

private:
    void InitializeAudioTypeVolumes();
    void CleanupInactiveComponents();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location);
};