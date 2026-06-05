#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dinosaur,
    Environment,
    Combat,
    UI,
    Narration
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
    bool bIs3D = true;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound2D(const FString& SoundID, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound3D(const FString& SoundID, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAttached(const FString& SoundID, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FString& SoundID, USoundBase* SoundAsset, EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundID) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<FString, FAudio_SoundEntry> SoundRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<FString, UAudioComponent*> ActiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float MasterVolume = 1.0f;

private:
    void LoadDefaultSounds();
    float GetEffectiveVolume(const FAudio_SoundEntry& SoundEntry, float VolumeMultiplier) const;
};