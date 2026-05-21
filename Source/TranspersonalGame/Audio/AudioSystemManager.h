#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AudioType : uint8
{
    SFX,
    Music,
    Ambient,
    Voice,
    UI
};

UENUM(BlueprintType)
enum class EAudio_BiomeAmbient : uint8
{
    Savana,
    Forest,
    Desert,
    Mountain,
    Swamp
};

USTRUCT(BlueprintType)
struct FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_AudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime;

    FAudio_SoundData()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        FadeInTime = 0.0f;
        FadeOutTime = 0.0f;
        AudioType = EAudio_AudioType::SFX;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ProximityAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    TSoftObjectPtr<USoundBase> ProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    bool bUseDistanceAttenuation;

    FAudio_ProximityAudioData()
    {
        TriggerDistance = 1000.0f;
        MaxDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
        bUseDistanceAttenuation = true;
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

    // Core audio playback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySound2D(USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, bool bPersistent = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySoundAtLocation(const UObject* WorldContext, USoundBase* Sound, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.0f);

    // Proximity audio system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterProximityAudio(AActor* SourceActor, const FAudio_ProximityAudioData& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterProximityAudio(AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateProximityAudio(APawn* Listener);

    // Biome ambient audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiomeAmbient(EAudio_BiomeAmbient BiomeType, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopBiomeAmbient(float FadeTime = 2.0f);

    // Damage audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDamageAudio(float DamageAmount, FVector HitLocation);

    // T-Rex proximity audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayTRexFootstepAudio(FVector TRexLocation, float Intensity = 1.0f);

    // Day/Night cycle audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToDayAudio(float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToNightAudio(float TransitionTime = 5.0f);

    // Master volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbientVolume(float Volume);

protected:
    // Audio component management
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, FAudio_ProximityAudioData> ProximityAudioMap;

    // Biome ambient audio
    UPROPERTY()
    UAudioComponent* CurrentBiomeAmbient;

    UPROPERTY()
    TMap<EAudio_BiomeAmbient, TSoftObjectPtr<USoundBase>> BiomeAmbientSounds;

    // Damage audio
    UPROPERTY()
    TSoftObjectPtr<USoundBase> DamageAudioSound;

    // T-Rex audio
    UPROPERTY()
    TSoftObjectPtr<USoundBase> TRexFootstepSound;

    // Day/Night audio
    UPROPERTY()
    UAudioComponent* DayAmbientComponent;

    UPROPERTY()
    UAudioComponent* NightAmbientComponent;

    UPROPERTY()
    TSoftObjectPtr<USoundBase> DayAmbientSound;

    UPROPERTY()
    TSoftObjectPtr<USoundBase> NightAmbientSound;

    // Volume settings
    UPROPERTY()
    float MasterVolume;

    UPROPERTY()
    float SFXVolume;

    UPROPERTY()
    float MusicVolume;

    UPROPERTY()
    float AmbientVolume;

private:
    void CleanupFinishedComponents();
    void LoadDefaultAudioAssets();
    float CalculateDistanceAttenuation(float Distance, float MaxDistance) const;
};