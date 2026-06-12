#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dinosaur,
    Player,
    Weather,
    Combat,
    UI
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category = EAudio_SoundCategory::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    FAudio_SoundConfig()
    {
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        Category = EAudio_SoundCategory::Ambient;
        bIs3D = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FAudio_SoundConfig& SoundConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FAudio_SoundConfig& SoundConfig, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSoundsOfCategory(EAudio_SoundCategory Category);

protected:
    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    float MasterVolume = 1.0f;

private:
    void InitializeDefaultVolumes();
    void CleanupFinishedComponents();
};

#include "AudioSystemManager.generated.h"