#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "Audio_AudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 1000.0f;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("DefaultSound");
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
        AttenuationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundEntry> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EBiomeType::Savanna;
        MusicVolume = 0.3f;
        AmbientVolume = 0.7f;
    }
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Extreme     UMETA(DisplayName = "Extreme Threat")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* SFXAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TArray<FAudio_BiomeAudioConfig> BiomeAudioConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    TMap<FString, FAudio_SoundEntry> SFXLibrary;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VoiceVolume = 1.0f;

private:
    float ThreatLevelTimer;
    float BiomeTransitionTimer;
    bool bIsTransitioningBiome;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySFX(const FString& SFXName, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoiceLine(USoundBase* VoiceClip, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSFXVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetVoiceVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSFX(const FString& SFXName, USoundBase* SoundAsset, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeBiomeAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EBiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

private:
    void UpdateMusicForThreatLevel();
    void TransitionBiomeAudio();
    void LoadBiomeAudioAssets();
    FAudio_BiomeAudioConfig* GetBiomeConfig(EBiomeType BiomeType);
};