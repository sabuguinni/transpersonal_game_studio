#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "../Core/SharedTypes.h"
#include "AudioSystemManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO SYSTEM MANAGER
 * Audio Agent #16
 * 
 * Manages all audio systems for the prehistoric survival game:
 * - Environmental audio (wind, water, forest ambience)
 * - Creature sounds (dinosaur roars, footsteps, breathing)
 * - Interactive audio (crafting, combat, survival actions)
 * - Dynamic music system based on danger level and location
 * - 3D positional audio for immersive prehistoric world
 */

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Environment = 0     UMETA(DisplayName = "Environment"),
    Creatures = 1       UMETA(DisplayName = "Creatures"),
    Combat = 2          UMETA(DisplayName = "Combat"),
    Crafting = 3        UMETA(DisplayName = "Crafting"),
    Music = 4           UMETA(DisplayName = "Music"),
    UI = 5              UMETA(DisplayName = "UI"),
    Narration = 6       UMETA(DisplayName = "Narration")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Tense = 2           UMETA(DisplayName = "Tense"),
    Dangerous = 3       UMETA(DisplayName = "Dangerous"),
    Lethal = 4          UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category = EAudio_SoundCategory::Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    FAudio_SoundData()
    {
        Category = EAudio_SoundCategory::Environment;
        Volume = 1.0f;
        Pitch = 1.0f;
        AttenuationDistance = 1000.0f;
        bIs3D = true;
        bLooping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float IntensityLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TimeOfDay = 12.0f; // 0-24 hours

    FAudio_MusicState()
    {
        DangerLevel = EAudio_DangerLevel::Safe;
        CurrentBiome = EEng_BiomeType::Forest;
        IntensityLevel = 0.0f;
        bInCombat = false;
        bNearWater = false;
        TimeOfDay = 12.0f;
    }
};

/**
 * Audio System Manager - Handles all audio functionality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound3D(const FAudio_SoundData& SoundData, const FVector& Location, AActor* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound2D(const FAudio_SoundData& SoundData, AActor* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSoundsOfCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateEnvironmentalAudio(const FVector& PlayerLocation, EEng_BiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayWindSound(float WindIntensity, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayWaterSound(const FVector& Location, bool bIsRiver = true);

    // Creature Audio
    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void PlayCreatureFootsteps(const FVector& Location, float CreatureSize, float MovementSpeed);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void PlayCreatureRoar(const FVector& Location, float CreatureSize, EAudio_DangerLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void PlayCreatureBreathing(const FVector& Location, float CreatureSize, bool bIsAggressive);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void UpdateMusicState(const FAudio_MusicState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetDangerLevel(EAudio_DangerLevel NewDangerLevel);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void TransitionToMusic(const FString& MusicTrackName, float FadeTime = 2.0f);

    // Combat Audio
    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void PlayWeaponSound(const FVector& Location, const FString& WeaponType);

    UFUNCTION(BlueprintCallable, Category = "Combat Audio")
    void PlayImpactSound(const FVector& Location, const FString& MaterialType);

    // Crafting Audio
    UFUNCTION(BlueprintCallable, Category = "Crafting Audio")
    void PlayCraftingSound(const FVector& Location, const FString& CraftingAction);

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetSFXVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Settings")
    void SetVoiceVolume(float Volume);

protected:
    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_SoundCategory, UAudioComponent*> CategoryAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent;

    // Audio Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<FString, FAudio_SoundData> SoundDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<EEng_BiomeType, TArray<FAudio_SoundData>> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    TMap<EAudio_DangerLevel, TArray<FAudio_SoundData>> DangerLevelMusic;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolume = 1.0f;

    // Internal Functions
    void InitializeAudioComponents();
    void InitializeSoundDatabase();
    void InitializeBiomeAmbientSounds();
    void InitializeDangerLevelMusic();
    
    UAudioComponent* GetOrCreateAudioComponent(EAudio_SoundCategory Category);
    void UpdateAmbienceForBiome(EEng_BiomeType Biome, const FVector& Location);
    void CrossfadeMusic(USoundCue* NewMusic, float FadeTime);
};