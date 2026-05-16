#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Footsteps   UMETA(DisplayName = "Footsteps"),
    Creatures   UMETA(DisplayName = "Creatures"),
    Weather     UMETA(DisplayName = "Weather"),
    UI          UMETA(DisplayName = "UI"),
    Narration   UMETA(DisplayName = "Narration")
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TObjectPtr<USoundCue>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RandomSoundInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RandomSoundChance = 0.3f;

    FAudio_BiomeAudioConfig()
    {
        AmbientLoop = nullptr;
        AmbientVolume = 0.7f;
        RandomSoundInterval = 15.0f;
        RandomSoundChance = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchVariation = 0.2f;

    FAudio_FootstepConfig()
    {
        FootstepSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.2f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> EffectsAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    // Biome Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioConfig> BiomeAudioConfigs;

    // Footstep Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    TMap<FString, FAudio_FootstepConfig> FootstepConfigs;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsAmbientPlaying;

    // Timer Handles
    FTimerHandle RandomSoundTimer;
    FTimerHandle BiomeCheckTimer;

public:
    // Main Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayAmbientAudio(EAudio_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayFootstepSound(const FString& SurfaceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayCreatureSound(TObjectPtr<USoundCue> CreatureSound, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarration(TObjectPtr<USoundCue> NarrationSound);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopNarration();

    // Biome Detection
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    EAudio_BiomeType DetectCurrentBiome(const FVector& PlayerLocation);

    // Volume Controls
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetEffectsVolume(float Volume);

protected:
    // Internal Functions
    void PlayRandomBiomeSound();
    void CheckBiomeChange();
    void InitializeBiomeConfigs();
    void InitializeFootstepConfigs();

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EffectsVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float NarrationVolume = 0.9f;
};