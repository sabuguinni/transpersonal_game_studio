#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Music,
    SFX,
    Ambience,
    Dialogue,
    UI
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
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool b3D = true;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
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
    void PlaySound(const FString& SoundID, AActor* SourceActor = nullptr, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void LoadSoundLibrary();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PauseAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ResumeAllSounds();

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
    float GetEffectiveVolume(const FAudio_SoundEntry& SoundEntry) const;
};

#include "Audio_SoundManager.generated.h"