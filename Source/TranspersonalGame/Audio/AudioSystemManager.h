#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Music,
    SFX,
    Voice,
    UI
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category = EAudio_SoundCategory::SFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 0.0f;

    FAudio_SoundConfig()
    {
        SoundCue = nullptr;
        Category = EAudio_SoundCategory::SFX;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        bLooping = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FAudio_SoundConfig& SoundConfig);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FAudio_SoundConfig& SoundConfig, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetMasterVolume() const { return MasterVolume; }

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

private:
    void InitializeDefaultVolumes();
    void CleanupFinishedComponents();
};

#include "AudioSystemManager.generated.h"