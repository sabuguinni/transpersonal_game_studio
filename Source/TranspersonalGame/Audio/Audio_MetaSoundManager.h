#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dialogue,
    Music,
    SFX,
    Narration
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 0.0f;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TArray<FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio System")
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float SFXVolume = 0.8f;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSound(const FString& SoundID, EAudio_SoundCategory Category, USoundBase* SoundAsset, 
                      float Volume = 1.0f, float Pitch = 1.0f, bool bLooping = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool PlaySound(const FString& SoundID, FVector Location = FVector::ZeroVector, bool bAttachToActor = false, AActor* ActorToAttachTo = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundID, bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds(EAudio_SoundCategory Category = EAudio_SoundCategory::SFX, bool bFadeOut = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAmbientLoop(const FString& AmbientID, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarration(const FString& NarrationID);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsSoundPlaying(const FString& SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializePrehistoricAudio();

private:
    FAudio_SoundEntry* FindSoundEntry(const FString& SoundID);
    float GetVolumeForCategory(EAudio_SoundCategory Category) const;
    void CleanupFinishedComponents();
};