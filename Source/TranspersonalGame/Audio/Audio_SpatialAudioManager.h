#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_SpatialAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SpatialSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIsLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bIs3D;

    FAudio_SpatialSoundData()
    {
        WorldLocation = FVector::ZeroVector;
        MaxAudibleDistance = 2000.0f;
        VolumeMultiplier = 1.0f;
        bIsLooping = false;
        bIs3D = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundCue>> RandomSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float RandomSoundFrequency;

    FAudio_BiomeAudioSettings()
    {
        AmbientVolume = 0.7f;
        MusicVolume = 0.5f;
        RandomSoundFrequency = 15.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AAudio_SpatialAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SpatialAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componente de áudio principal
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MasterAudioComponent;

    // Componentes de áudio para diferentes canais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* EffectsAudioComponent;

    // Configurações de áudio por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<FString, FAudio_BiomeAudioSettings> BiomeAudioSettings;

    // Sons espaciais activos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    TArray<FAudio_SpatialSoundData> ActiveSpatialSounds;

    // Referência ao jogador
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    APawn* PlayerPawn;

    // Estado actual do sistema
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    FString CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    float CurrentThreatLevel;

    // Timers para sons aleatórios
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    float RandomSoundTimer;

public:
    // Métodos principais
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlaySpatialSound(const FAudio_SpatialSoundData& SoundData);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void StopSpatialSound(FVector Location, float Tolerance = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetCurrentBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdatePlayerReference();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeBiomeAudioSettings();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    FString DetectCurrentBiome(FVector PlayerLocation);

protected:
    // Métodos internos
    void UpdateSpatialAudio();
    void UpdateBiomeAudio();
    void ProcessRandomSounds(float DeltaTime);
    void CleanupFinishedSounds();
    float CalculateVolumeFromDistance(FVector SoundLocation, FVector ListenerLocation, float MaxDistance);
};