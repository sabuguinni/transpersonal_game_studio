#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio/AudioManager.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - AUDIO MANAGER
 * Audio Agent #16
 * 
 * Central audio management system for the prehistoric survival game.
 * Handles ambient sounds, music layers, and dynamic audio responses.
 * Focus on realistic prehistoric soundscapes and tension building.
 */

UENUM(BlueprintType)
enum class EAudio_BiomeAmbience : uint8
{
    Forest = 0          UMETA(DisplayName = "Forest"),
    Swamp = 1           UMETA(DisplayName = "Swamp"),
    Grassland = 2       UMETA(DisplayName = "Grassland"),
    Desert = 3          UMETA(DisplayName = "Desert"),
    Mountains = 4       UMETA(DisplayName = "Mountains"),
    River = 5           UMETA(DisplayName = "River"),
    Cave = 6            UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_TensionLevel : uint8
{
    Calm = 0            UMETA(DisplayName = "Calm"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Alert = 2           UMETA(DisplayName = "Alert"),
    Danger = 3          UMETA(DisplayName = "Danger"),
    Terror = 4          UMETA(DisplayName = "Terror")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn = 0            UMETA(DisplayName = "Dawn"),
    Morning = 1         UMETA(DisplayName = "Morning"),
    Noon = 2            UMETA(DisplayName = "Noon"),
    Afternoon = 3       UMETA(DisplayName = "Afternoon"),
    Dusk = 4            UMETA(DisplayName = "Dusk"),
    Night = 5           UMETA(DisplayName = "Night"),
    Midnight = 6        UMETA(DisplayName = "Midnight")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FAudio_SoundLayer()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = true;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeAmbience BiomeType = EAudio_BiomeAmbience::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_SoundLayer BaseAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_SoundLayer DayVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_SoundLayer NightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundLayer> RandomElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float RandomElementChance = 0.1f;

    FAudio_BiomeSettings()
    {
        BiomeType = EAudio_BiomeAmbience::Forest;
        RandomElementChance = 0.1f;
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

    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeAmbience NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTensionLevel(EAudio_TensionLevel NewTension);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(const FString& SurfaceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayCraftingSound(const FString& MaterialType, const FVector& Location);

    // Dynamic Audio Response
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnPlayerHealthChanged(float HealthPercentage);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnPlayerFearChanged(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void OnDinosaurNearby(const FString& DinosaurType, float Distance);

    // Audio Configuration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetEffectsVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float Volume);

    // Audio State Queries
    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_BiomeAmbience GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_TensionLevel GetCurrentTension() const { return CurrentTension; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

protected:
    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_BiomeAmbience CurrentBiome = EAudio_BiomeAmbience::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_TensionLevel CurrentTension = EAudio_TensionLevel::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Morning;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> AmbienceComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> TensionComponent;

    // Volume Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbienceVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float EffectsVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.6f;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_BiomeAmbience, FAudio_BiomeSettings> BiomeSettings;

    // Tension Music Layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<EAudio_TensionLevel, FAudio_SoundLayer> TensionLayers;

    // Sound Effect Libraries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, TSoftObjectPtr<USoundCue>> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, TSoftObjectPtr<USoundCue>> FootstepSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TMap<FString, TSoftObjectPtr<USoundCue>> CraftingSounds;

    // Internal Methods
    void UpdateAmbienceLayer();
    void UpdateTensionLayer();
    void UpdateMusicLayer();
    void PlayRandomAmbienceElement();

    // Timer Handles
    FTimerHandle RandomElementTimer;
    FTimerHandle TensionUpdateTimer;
};