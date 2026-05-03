#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "AudioSubsystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FAudio_BiomeAudioData()
    {
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> MovementSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> ThreatSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxHearingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    FAudio_DinosaurAudioData()
    {
        MaxHearingDistance = 5000.0f;
        Volume = 1.0f;
    }
};

/**
 * Sistema central de áudio do jogo.
 * Gere música ambiental por bioma, efeitos sonoros de dinossauros,
 * e áudio contextual baseado na localização e ameaças.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSubsystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EBiomeType GetCurrentBiome() const { return CurrentBiome; }

    // Threat level audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(EDinosaurSpecies Species, const FVector& Location, bool bIsThreat = false);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(EDinosaurSpecies Species, const FVector& Location, float VolumeMultiplier = 1.0f);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalSound(const FString& SoundName, const FVector& Location, float Volume = 1.0f);

    // Master volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetEffectsVolume(float Volume);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EThreatLevel CurrentThreatLevel;

    // Audio data maps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EDinosaurSpecies, FAudio_DinosaurAudioData> DinosaurAudioData;

    // Active audio components
    UPROPERTY()
    UAudioComponent* CurrentAmbientAudio;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveEffectAudio;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
    float EffectsVolume;

    // Internal methods
    void InitializeAudioData();
    void TransitionAmbientAudio(EBiomeType NewBiome);
    void CleanupFinishedAudio();
    USoundCue* GetRandomDinosaurSound(EDinosaurSpecies Species, bool bIsThreat = false);

    // Timer handles
    FTimerHandle CleanupTimerHandle;
};