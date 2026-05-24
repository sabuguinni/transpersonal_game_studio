#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "AudioManager.generated.h"

/**
 * AUDIO MANAGER - TRANSPERSONAL GAME STUDIO
 * Agent #16 - Audio Agent
 * 
 * Manages all audio in the prehistoric survival game:
 * - Environmental ambience (forest, wind, water)
 * - Dinosaur sounds (roars, footsteps, breathing)
 * - Player feedback (footsteps, crafting, damage)
 * - Dynamic music system based on threat level
 * - 3D spatial audio for immersive prehistoric world
 */

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambience = 0        UMETA(DisplayName = "Ambience"),
    DinosaurVocal = 1   UMETA(DisplayName = "Dinosaur Vocal"),
    DinosaurMovement = 2 UMETA(DisplayName = "Dinosaur Movement"),
    PlayerAction = 3    UMETA(DisplayName = "Player Action"),
    Environment = 4     UMETA(DisplayName = "Environment"),
    Music = 5           UMETA(DisplayName = "Music"),
    UI = 6              UMETA(DisplayName = "UI")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Danger = 2          UMETA(DisplayName = "Danger"),
    Combat = 3          UMETA(DisplayName = "Combat"),
    Panic = 4           UMETA(DisplayName = "Panic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_SoundCategory Category = EAudio_SoundCategory::Ambience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    FAudio_SoundDefinition()
    {
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::Ambience;
        Volume = 1.0f;
        Pitch = 1.0f;
        MaxDistance = 5000.0f;
        bIs3D = true;
        bLooping = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float IntensityLevel = 0.0f;

    FAudio_MusicState()
    {
        CurrentThreatLevel = EAudio_ThreatLevel::Safe;
        CurrentBiome = TEXT("Forest");
        TimeOfDay = 12.0f;
        bInCombat = false;
        IntensityLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound2D(const FString& SoundName, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound3D(const FString& SoundName, const FVector& Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Music")
    void UpdateMusicState(EAudio_ThreatLevel ThreatLevel, const FString& Biome, float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetCombatMusic(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void FadeToMusic(const FString& MusicName, float FadeTime = 2.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetAmbienceForBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PlayFootstepSound(const FString& SurfaceType, const FVector& Location, bool bIsHeavy = false);

    // Audio Settings
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    float GetCategoryVolume(EAudio_SoundCategory Category) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsSoundPlaying(const FString& SoundName) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSoundDefinition(const FAudio_SoundDefinition& SoundDef);

protected:
    // Sound Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, FAudio_SoundDefinition> SoundRegistry;

    // Active Audio Components
    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    // Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    UAudioComponent* AmbienceComponent;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

private:
    void InitializeDefaultSounds();
    void InitializeCategoryVolumes();
    void UpdateDynamicMusic();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundDefinition& SoundDef, const FVector& Location);
    float CalculateFinalVolume(const FAudio_SoundDefinition& SoundDef, float VolumeMultiplier) const;
};