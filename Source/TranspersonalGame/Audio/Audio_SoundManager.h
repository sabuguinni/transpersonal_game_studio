#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "Subsystems/GameInstanceSubsystem.h"
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
struct TRANSPERSONALGAME_API FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category;

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

    FAudio_SoundData()
    {
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

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Sound playback functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound2D(USoundCue* SoundCue, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySoundAttached(USoundCue* SoundCue, USceneComponent* AttachComponent, float Volume = 1.0f, float Pitch = 1.0f);

    // Category-based sound management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void MuteCategoryToggle(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSoundsInCategory(EAudio_SoundCategory Category);

    // Ambient sound management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAmbientLoop(USoundCue* AmbientSound, float Volume = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CrossfadeAmbient(USoundCue* NewAmbientSound, float CrossfadeTime = 3.0f);

    // Dynamic audio based on game state
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioForDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioForWeather(const FString& WeatherType);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, FVector Location, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FString& SurfaceType, FVector Location, float Weight);

protected:
    // Category volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudio_SoundCategory, bool> CategoryMuted;

    // Active audio components
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    // Sound libraries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Libraries")
    TMap<FString, FAudio_SoundData> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Libraries")
    TMap<FString, FAudio_SoundData> EnvironmentSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Libraries")
    TMap<FString, FAudio_SoundData> AmbientSounds;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    int32 MaxConcurrentSounds = 32;

private:
    void CleanupFinishedComponents();
    void InitializeSoundLibraries();
    UAudioComponent* CreateAudioComponent(USoundCue* SoundCue, float Volume, float Pitch);
};