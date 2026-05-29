#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Pantano"),
    Forest      UMETA(DisplayName = "Floresta"), 
    Savanna     UMETA(DisplayName = "Savana"),
    Desert      UMETA(DisplayName = "Deserto"),
    Mountain    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Seguro"),
    Caution     UMETA(DisplayName = "Cuidado"),
    Danger      UMETA(DisplayName = "Perigo"),
    Critical    UMETA(DisplayName = "Crítico")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float WindIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<TSoftObjectPtr<USoundCue>> RandomSounds;

    FAudio_BiomeAudioSettings()
    {
        BaseVolume = 0.7f;
        WindIntensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TSoftObjectPtr<USoundCue> ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float TriggerDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    float VolumeMultiplier = 1.0f;

    FAudio_ThreatAudioData()
    {
        TriggerDistance = 1000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AdaptiveSoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AdaptiveSoundManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes de áudio
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ThreatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* WeatherAudioComponent;

    // Configurações por bioma
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EAudio_BiomeType, FAudio_BiomeAudioSettings> BiomeAudioSettings;

    // Configurações de ameaça
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Settings")
    TMap<EAudio_ThreatLevel, FAudio_ThreatAudioData> ThreatAudioSettings;

    // Estado actual
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentWeatherIntensity;

    // Timers para transições
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float BiomeTransitionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float ThreatFadeTimer;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateBiomeAudio(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateWeatherIntensity(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void PlayRandomBiomeSound();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    EAudio_BiomeType DetectCurrentBiome(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    EAudio_ThreatLevel CalculateThreatLevel();

protected:
    // Funções internas
    void InitializeBiomeSettings();
    void InitializeThreatSettings();
    void UpdateAudioComponents(float DeltaTime);
    void HandleBiomeTransition(float DeltaTime);
    void HandleThreatTransition(float DeltaTime);

    // Timer para sons aleatórios
    UPROPERTY()
    float RandomSoundTimer;

    UPROPERTY(EditAnywhere, Category = "Audio Settings")
    float RandomSoundInterval = 15.0f;
};