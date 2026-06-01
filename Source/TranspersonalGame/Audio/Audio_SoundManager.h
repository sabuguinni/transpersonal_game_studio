#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    DinosaurVocal,
    PlayerFootsteps,
    Crafting,
    Weather,
    Combat,
    UI
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDistance = 5000.0f;

    FAudio_SoundEntry()
    {
        Category = EAudio_SoundCategory::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        MaxDistance = 5000.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Sound playback functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(const FString& SoundName, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound2D(const FString& SoundName, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAttached(const FString& SoundName, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

    // Ambient sound management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAmbientLoop(const FString& SoundName, FVector Location, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop(const FString& SoundName, float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllAmbientLoops(float FadeOutTime = 2.0f);

    // Sound registration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FString& SoundName, const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterSound(const FString& SoundName);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Distance-based volume calculation
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float CalculateVolumeFromDistance(float Distance, float MaxDistance) const;

protected:
    // Sound registry
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    // Active ambient loops
    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAmbientLoops;

    // Category volume multipliers
    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Helper functions
    FAudio_SoundEntry* GetSoundEntry(const FString& SoundName);
    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();
};