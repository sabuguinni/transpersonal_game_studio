#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundType : uint8
{
    Ambient         UMETA(DisplayName = "Ambient"),
    DinosaurCall    UMETA(DisplayName = "Dinosaur Call"),
    Footsteps       UMETA(DisplayName = "Footsteps"),
    Weather         UMETA(DisplayName = "Weather"),
    Combat          UMETA(DisplayName = "Combat"),
    UI              UMETA(DisplayName = "UI"),
    Narration       UMETA(DisplayName = "Narration")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundWave> SoundWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bShouldLoop;

    FAudio_SoundData()
    {
        SoundType = EAudio_SoundType::Ambient;
        VolumeMultiplier = 1.0f;
        PitchMultiplier = 1.0f;
        AttenuationRadius = 1000.0f;
        bShouldLoop = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EAudio_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_SoundData> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_SoundData> AggressiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_SoundData> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_SoundData> DeathSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float CallFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float ProximityTriggerDistance;

    FAudio_DinosaurAudioProfile()
    {
        Species = EAudio_DinosaurSpecies::Generic;
        CallFrequency = 30.0f;
        ProximityTriggerDistance = 1500.0f;
    }
};

/**
 * Prehistoric Audio System Manager
 * Handles all audio in the prehistoric survival game including:
 * - Ambient forest/weather sounds
 * - Dinosaur calls and proximity audio
 * - Dynamic music based on threat level
 * - Spatial audio positioning
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FAudio_SoundData& SoundData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FAudio_SoundData& SoundData, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(EAudio_SoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSoundTypeVolume(EAudio_SoundType SoundType, float Volume);

    // Dinosaur audio system
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurCall(EAudio_DinosaurSpecies Species, const FVector& Location, bool bIsAggressive = false);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurFootstep(EAudio_DinosaurSpecies Species, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaurActor(AActor* DinosaurActor, EAudio_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UnregisterDinosaurActor(AActor* DinosaurActor);

    // Ambient audio system
    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void SetAmbientWeather(const FString& WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void UpdateThreatLevel(float ThreatLevel);

    // Audio analysis
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    int32 GetActiveAudioComponentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    TArray<FString> GetActiveSoundTypes() const;

protected:
    // Audio component management
    UPROPERTY()
    TMap<EAudio_SoundType, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    TMap<AActor*, UAudioComponent*> DinosaurAudioComponents;

    // Audio profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles")
    TMap<EAudio_DinosaurSpecies, FAudio_DinosaurAudioProfile> DinosaurAudioProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles")
    TMap<FString, FAudio_SoundData> AmbientSounds;

    // Volume settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Settings")
    TMap<EAudio_SoundType, float> SoundTypeVolumes;

    // Dynamic audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FString CurrentWeather;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

private:
    // Internal audio management
    UAudioComponent* CreateAudioComponent(const FAudio_SoundData& SoundData, const FVector& Location = FVector::ZeroVector);
    void CleanupInactiveComponents();
    void InitializeDefaultProfiles();
    void LoadAudioAssets();

    // Timers
    FTimerHandle AmbientUpdateTimer;
    FTimerHandle DinosaurCallTimer;
    FTimerHandle ComponentCleanupTimer;

    // Audio asset references
    UPROPERTY()
    TMap<FString, USoundCue*> LoadedSoundCues;

    UPROPERTY()
    TMap<FString, USoundWave*> LoadedSoundWaves;
};