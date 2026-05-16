#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "AudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    class USoundCue* AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<class USoundWave*> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundChance;

    FAudio_BiomeAudioData()
    {
        AmbientLoop = nullptr;
        AmbientVolume = 0.7f;
        RandomSoundChance = 0.1f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DynamicAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    EBiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    bool bIsNightTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float WeatherIntensity;

    FAudio_DynamicAudioState()
    {
        DangerLevel = 0.0f;
        CurrentBiome = EBiomeType::Savanna;
        bIsNightTime = false;
        WeatherIntensity = 0.0f;
    }
};

/**
 * Audio Manager - Handles biome-adaptive soundscapes and dynamic audio
 * Provides prehistoric world audio with environmental storytelling
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateBiomeAudio(EBiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetDangerLevel(float DangerAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarrativeClip(const FString& ClipName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateDynamicAudioState(const FAudio_DynamicAudioState& NewState);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayFootstepSound(EBiomeType BiomeType, const FVector& Location, bool bIsHeavy = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurRoar(const FString& DinosaurType, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StartWeatherAudio(EWeatherType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void StopWeatherAudio();

    // Narrative Audio
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySurvivalTip(const FString& TipCategory);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDangerWarning(const FString& ThreatType, const FVector& ThreatLocation);

protected:
    // Audio Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Data")
    TMap<EBiomeType, FAudio_BiomeAudioData> BiomeAudioMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Data")
    TMap<FString, class USoundWave*> NarrativeClips;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Data")
    TMap<FString, class USoundWave*> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Data")
    TMap<EBiomeType, class USoundWave*> FootstepSounds;

    // Audio Components
    UPROPERTY()
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY()
    class UAudioComponent* NarrativeAudioComponent;

    // State
    UPROPERTY()
    FAudio_DynamicAudioState CurrentAudioState;

    UPROPERTY()
    float LastBiomeTransitionTime;

private:
    void InitializeAudioData();
    void CreateAudioComponents();
    void UpdateAmbientAudio();
    void CrossfadeBiomeAudio(EBiomeType FromBiome, EBiomeType ToBiome);
    
    UPROPERTY()
    class UWorld* CachedWorld;
};