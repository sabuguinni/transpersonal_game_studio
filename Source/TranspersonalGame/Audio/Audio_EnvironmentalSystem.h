#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_EnvironmentalSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Water       UMETA(DisplayName = "Water"),
    Cave        UMETA(DisplayName = "Cave"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    Safe        UMETA(DisplayName = "Safe Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 1.5f;

    FAudio_BiomeAudioSet()
    {
        BaseVolume = 0.6f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> MovementSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> AlertSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> AttackSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxHearingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components for different layers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SFXAudioComponent;

    // Biome Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioSet> BiomeAudioSets;

    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TransitionProgress;

    // Audio Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<ATriggerVolume*> AudioTriggerVolumes;

    // Dinosaur Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<FString, FAudio_DinosaurSoundSet> DinosaurSoundSets;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AudioUpdateInterval = 0.1f;

    // Distance-based audio attenuation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MinAudioDistance = 100.0f;

private:
    float LastAudioUpdateTime;
    APawn* PlayerPawn;

public:
    // Audio Management Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetBiomeAudio(EAudio_BiomeType NewBiome, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void RegisterAudioTrigger(ATriggerVolume* TriggerVolume, EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdatePlayerAudioZone();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    bool IsTransitioning() const { return bIsTransitioning; }

protected:
    void InitializeAudioComponents();
    void InitializeBiomeAudioSets();
    void InitializeDinosaurSoundSets();
    void UpdateAudioTransition(float DeltaTime);
    void CheckAudioTriggers();
    float CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation);
    void ApplyVolumeSettings();
};