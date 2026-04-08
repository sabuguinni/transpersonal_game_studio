#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioModulationStatics.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System Architecture for Jurassic Survival Game
 * 
 * Philosophy: The sound that doesn't exist is often more powerful than the sound that exists.
 * When audio is perfect, the player doesn't think about audio — they think about the world.
 * 
 * This system creates constant tension through:
 * - Adaptive music that responds to threat levels
 * - Procedural ambient soundscapes
 * - Individual dinosaur audio signatures
 * - Spatial audio that makes the player feel like prey
 */

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"), 
    Danger      UMETA(DisplayName = "Danger"),
    Panic       UMETA(DisplayName = "Panic"),
    Stealth     UMETA(DisplayName = "Stealth")
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveSystem      UMETA(DisplayName = "Cave System"),
    Swampland       UMETA(DisplayName = "Swampland")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny        UMETA(DisplayName = "Tiny (< 1m)"),
    Small       UMETA(DisplayName = "Small (1-3m)"),
    Medium      UMETA(DisplayName = "Medium (3-8m)"),
    Large       UMETA(DisplayName = "Large (8-15m)"),
    Massive     UMETA(DisplayName = "Massive (> 15m)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType CurrentEnvironment = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeartRate = 60.0f; // Simulated stress level
};

/**
 * Master Audio System Manager
 * Coordinates all audio subsystems and maintains global audio state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core System Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaur(class ADinosaurCharacter* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterDinosaur(class ADinosaurCharacter* Dinosaur);

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void TransitionToMusicState(const FString& StateName, float TransitionTime = 2.0f);

    // Ambient System
    UFUNCTION(BlueprintCallable, Category = "Ambient System")
    void UpdateEnvironmentAmbience(EEnvironmentType Environment);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlayDinosaurSound(class ADinosaurCharacter* Dinosaur, const FString& SoundType);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioState CurrentAudioState;

    // MetaSound Sources for adaptive systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> EnvironmentAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurAudioMetaSound;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    // Registered dinosaurs for audio tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<class ADinosaurCharacter>> RegisteredDinosaurs;

private:
    void UpdateMusicSystem();
    void UpdateAmbienceSystem();
    void UpdateSpatialAudio();
    void CalculateThreatLevel();
};

/**
 * Dinosaur Audio Component
 * Handles individual dinosaur audio signatures and behaviors
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

    // Dinosaur-specific audio properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    EDinosaurSize DinosaurSize = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FString SpeciesName = "Unknown";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VocalRange = 1000.0f; // Distance in UE units

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float AggressionLevel = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    bool bIsHerbivore = true;

    // Audio behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayFootstep();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayBreathing();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayVocalization(const FString& VocalizationType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayMovementSound(float Intensity);

protected:
    // MetaSound for procedural dinosaur audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurMetaSound;

    // Individual audio signature parameters
    UPROPERTY(BlueprintReadOnly, Category = "Audio Signature")
    float PitchVariation = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Signature")
    float VolumeVariation = 0.05f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Signature")
    float TimbreVariation = 0.15f;

private:
    void GenerateAudioSignature();
    void UpdateAudioParameters();
};