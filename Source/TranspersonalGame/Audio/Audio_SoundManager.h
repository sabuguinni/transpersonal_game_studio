#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambience,
    Music,
    SFX,
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
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool b3D = true;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        b3D = true;
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
    void PlaySound2D(const FString& SoundID, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound3D(const FString& SoundID, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void LoadSoundLibrary();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FAudio_SoundEntry> SoundLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, UAudioComponent*> ActiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

private:
    void InitializeDefaultSounds();
    float GetCategoryVolume(EAudio_SoundCategory Category) const;
};