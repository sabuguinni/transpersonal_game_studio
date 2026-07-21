#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Audio_AudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D;

    FAudio_SoundProfile()
    {
        SoundCue = nullptr;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        AttenuationRadius = 1000.0f;
        bIs3D = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FAudio_SoundProfile AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FAudio_SoundProfile DangerSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FAudio_SoundProfile WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EBiomeType BiomeType;

    FAudio_BiomeAudioSettings()
    {
        BiomeRadius = 2000.0f;
        BiomeType = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Manager")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Manager")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Manager")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Manager")
    UAudioComponent* SFXAudioComponent;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_BiomeAudioSettings> BiomeAudioSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume;

    // Dynamic Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsNightTime;

    // Audio Management Functions
    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlaySoundAtLocation(USoundCue* SoundCue, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayFootstepSound(FVector Location, ECreatureSize CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurRoar(FVector Location, EDinosaurSpecies Species, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateBiomeAudio(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateDangerLevel(float DangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetTimeOfDay(bool bNightTime);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayNarrationLine(const FString& NarrationText, float DelaySeconds = 0.0f);

    // MetaSound Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void TriggerMetaSoundEvent(const FString& EventName, float Parameter1 = 0.0f, float Parameter2 = 0.0f);

private:
    // Internal audio management
    void UpdateAmbientAudio();
    void UpdateMusicLayers();
    void ProcessAudioCulling();
    
    // Audio pools for performance
    TArray<UAudioComponent*> AudioComponentPool;
    int32 CurrentPoolIndex;
    
    // Cached references
    class ATranspersonalCharacter* PlayerCharacter;
    
    // Audio timing
    float LastAudioUpdate;
    float AudioUpdateInterval;
};