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
    None = 0,
    Ambient,
    Combat,
    Footsteps,
    DinosaurRoars,
    Weather,
    Narration
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bLooping;

    FAudio_SoundEntry()
    {
        Category = EAudio_SoundCategory::None;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
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
    void PlaySound(const FString& SoundName, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(const FString& SoundName, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FString& SoundName, const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FString& SurfaceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientLoop(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

private:
    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, const FVector& Location);
};