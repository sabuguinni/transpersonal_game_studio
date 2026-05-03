#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"), 
    Savana      UMETA(DisplayName = "Savana"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha Nevada")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ReverbIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float WindIntensity;

    FAudio_BiomeConfig()
    {
        AmbientSound = nullptr;
        BaseVolume = 0.5f;
        ReverbIntensity = 0.3f;
        WindIntensity = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bShouldLoop;

    FAudio_ThreatConfig()
    {
        ThreatSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        bShouldLoop = false;
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

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* ThreatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* MusicAudioComponent;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_BiomeConfig> BiomeConfigs;

    // Threat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TMap<EAudio_ThreatLevel, FAudio_ThreatConfig> ThreatConfigs;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float FadeTime;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayThreatSound(EAudio_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopThreatSound();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioBasedOnLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_BiomeType DetectBiomeFromLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float NewVolume);

protected:
    // Internal Functions
    void InitializeBiomeConfigs();
    void InitializeThreatConfigs();
    void UpdateAmbientAudio();
    void FadeToNewAmbient(USoundBase* NewSound, float TargetVolume);

    // Timers
    FTimerHandle BiomeUpdateTimer;
    void CheckBiomeUpdate();

    // Distance-based threat detection
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float CalculateThreatDistance();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Detection")
    TArray<AActor*> ThreatActors;
};