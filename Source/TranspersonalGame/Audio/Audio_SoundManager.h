#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Audio_SoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundWave> SoundWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 1000.0f;

    FAudio_SoundEntry()
    {
        SoundID = TEXT("");
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = false;
        AttenuationRadius = 1000.0f;
    }
};

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
    Footsteps,
    Weather
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
    void PlaySoundAtLocation(const FString& SoundID, const FVector& Location, float VolumeMultiplier = 1.0f);

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
    void RegisterSound(const FString& SoundID, USoundWave* SoundWave, EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSoundCue(const FString& SoundID, USoundCue* SoundCue, EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FString& SurfaceType, const FVector& Location, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientLoop(const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop();

protected:
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY()
    float MasterVolume;

    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry);
};