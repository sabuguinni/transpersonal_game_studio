#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "SharedTypes.h"
#include "AudioSystemManager.generated.h"

class UAudioComponent;
class USoundCue;
class USoundWave;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundWave> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationDistance;

    FAudio_SoundConfig()
    {
        SoundName = TEXT("");
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        bIs3D = true;
        AttenuationDistance = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundConfig> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CrossfadeTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BiomeRadius;

    FAudio_BiomeAmbience()
    {
        BiomeType = EBiomeType::Savana;
        CrossfadeTime = 2.0f;
        BiomeRadius = 5000.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Audio system management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioSystem(float DeltaTime);

    // Sound playback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySound2D(const FString& SoundName, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    UAudioComponent* PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    // Biome ambience
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCurrentBiome(EBiomeType NewBiome, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateBiomeAmbience(const FVector& PlayerLocation);

    // Footstep system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepSound(const FVector& Location, ESurfaceType SurfaceType, float MovementSpeed);

    // Damage feedback
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDamageSound(float DamageAmount, const FVector& ImpactLocation);

    // Dinosaur audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(EDinosaurSpecies Species, const FString& SoundType, const FVector& Location);

    // Screen shake audio trigger
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerScreenShakeAudio(float Intensity, const FVector& SourceLocation);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config")
    TArray<FAudio_SoundConfig> RegisteredSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Config")
    TArray<FAudio_BiomeAmbience> BiomeAmbienceConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    UAudioComponent* CurrentAmbienceComponent;

private:
    void LoadDefaultSounds();
    void SetupBiomeConfigs();
    FAudio_SoundConfig* FindSoundConfig(const FString& SoundName);
    void CrossfadeToBiome(EBiomeType NewBiome);
    float CalculateDistanceAttenuation(const FVector& SoundLocation, const FVector& ListenerLocation);
};