#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_EnvironmentalSoundManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundCue* AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<USoundCue*> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundVariance;

    FAudio_BiomeAudioData()
    {
        AmbientLoop = nullptr;
        RandomSoundInterval = 15.0f;
        RandomSoundVariance = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* RandomSoundComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<FString, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float VolumeMultiplier;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FString CurrentBiome;

    FTimerHandle RandomSoundTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopAllAmbientSounds();

protected:
    void StartRandomSoundTimer();
    void StopRandomSoundTimer();
    void InitializeBiomeAudioData();
};