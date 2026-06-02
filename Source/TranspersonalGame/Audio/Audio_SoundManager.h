#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    None = 0,
    Ambient,
    Music,
    SFX,
    Voice,
    UI,
    Dinosaur,
    Environment,
    Combat,
    Crafting
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category = EAudio_SoundCategory::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttenuationDistance = 1000.0f;

    FAudio_SoundConfig()
    {
        SoundAsset = nullptr;
        Category = EAudio_SoundCategory::None;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
        AttenuationDistance = 1000.0f;
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
    UAudioComponent* PlaySound2D(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAtLocation(USoundBase* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAttached(USoundBase* Sound, USceneComponent* AttachComponent, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSoundsByCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSoundConfig(const FString& SoundName, const FAudio_SoundConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlayConfiguredSound(const FString& SoundName, FVector Location = FVector::ZeroVector);

protected:
    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    TMap<FString, FAudio_SoundConfig> SoundConfigs;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    float MasterVolume = 1.0f;

    void CleanupFinishedComponents();
    float CalculateFinalVolume(EAudio_SoundCategory Category, float BaseVolume) const;
};