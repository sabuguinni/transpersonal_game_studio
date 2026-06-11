#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient,
    DinosaurFootsteps,
    DinosaurRoar,
    PlayerFootsteps,
    Fire,
    Weather,
    Combat,
    UI
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    FAudio_SoundEntry()
    {
        SoundType = EAudio_SoundType::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
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

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(EAudio_SoundType SoundType, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(EAudio_SoundType SoundType, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(EAudio_SoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSoundTypeVolume(EAudio_SoundType SoundType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSoundEntry(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurRoar(FVector Location, float IntensityLevel = 1.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> SoundEntries;

    UPROPERTY()
    TMap<EAudio_SoundType, UAudioComponent*> ActiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_SoundType, float> SoundTypeVolumes;

private:
    FAudio_SoundEntry* FindSoundEntry(EAudio_SoundType SoundType);
    void InitializeDefaultSounds();
};