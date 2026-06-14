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
    Ambient,
    Dinosaur,
    Player,
    Environment,
    UI,
    Music
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundName;

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
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime = 0.0f;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::Ambient;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Play sound functions
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    UAudioComponent* PlaySound2D(const FString& SoundName, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    UAudioComponent* PlaySoundAtLocation(const FString& SoundName, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    UAudioComponent* PlaySoundAttached(const FString& SoundName, USceneComponent* AttachComponent, float VolumeMultiplier = 1.0f);

    // Sound management
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopSoundsByCategory(EAudio_SoundCategory Category);

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Sound registration
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    bool IsSoundRegistered(const FString& SoundName) const;

    // Prehistoric-specific sounds
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurRoar(const FString& DinosaurType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayFootstepSound(const FString& SurfaceType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StartAmbientLoop(const FString& BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopAmbientLoop();

protected:
    // Sound registry
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    // Active audio components
    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveSounds;

    // Volume settings
    UPROPERTY()
    float MasterVolume = 1.0f;

    UPROPERTY()
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    // Current ambient loop
    UPROPERTY()
    UAudioComponent* CurrentAmbientLoop = nullptr;

    // Helper functions
    FAudio_SoundEntry* FindSoundEntry(const FString& SoundName);
    float CalculateFinalVolume(const FAudio_SoundEntry& SoundEntry, float VolumeMultiplier) const;
    void InitializeDefaultSounds();
    void CleanupFinishedSounds();
};