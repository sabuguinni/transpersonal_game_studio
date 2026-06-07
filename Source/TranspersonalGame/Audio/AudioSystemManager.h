#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioType : uint8
{
    Music,
    SFX,
    Ambience,
    Voice,
    UI
};

UENUM(BlueprintType)
enum class EAudio_AudioPriority : uint8
{
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FAudio_AudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbienceVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float UIVolume = 0.8f;

    FAudio_AudioSettings()
    {
        MasterVolume = 1.0f;
        MusicVolume = 0.8f;
        SFXVolume = 1.0f;
        AmbienceVolume = 0.6f;
        VoiceVolume = 1.0f;
        UIVolume = 0.8f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio playback functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound2D(USoundBase* Sound, EAudio_AudioType AudioType = EAudio_AudioType::SFX, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* PlaySound3D(USoundBase* Sound, FVector Location, EAudio_AudioType AudioType = EAudio_AudioType::SFX, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayMusic(USoundBase* MusicTrack, bool bLoop = true, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientSound(USoundBase* AmbienceSound, bool bLoop = true);

    // Volume control
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetVolumeByType(EAudio_AudioType AudioType, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetVolumeByType(EAudio_AudioType AudioType) const;

    // Audio settings
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ApplyAudioSettings(const FAudio_AudioSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FAudio_AudioSettings GetAudioSettings() const;

    // Dynamic audio system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateDynamicAudio(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDinosaurProximityAudio(float Distance, bool bIsLargeDinosaur = false);

protected:
    // Audio settings
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    FAudio_AudioSettings CurrentSettings;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbienceComponent;

    // Dynamic audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float CurrentThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    float LastDinosaurProximityTime = 0.0f;

    // Audio pools for performance
    UPROPERTY()
    TArray<UAudioComponent*> AudioComponentPool;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

private:
    void InitializeAudioComponentPool();
    UAudioComponent* GetPooledAudioComponent();
    void ReturnAudioComponentToPool(UAudioComponent* Component);
    float CalculateVolumeMultiplier(EAudio_AudioType AudioType) const;
};