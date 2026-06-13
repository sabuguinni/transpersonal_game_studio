#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Footsteps,
    DinosaurVocals,
    Combat,
    UI,
    Narration,
    Weather,
    Fire
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime = 0.0f;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
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
    void StopAllSoundsInCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void LoadSoundLibrary();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeInSound(const FString& SoundID, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeOutSound(const FString& SoundID, float FadeTime = 2.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> SoundLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveSounds;

    void InitializeDefaultSounds();
    void SetupCategoryVolumes();
};