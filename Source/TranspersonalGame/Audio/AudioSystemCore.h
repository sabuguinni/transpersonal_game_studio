#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemCore.generated.h"

/**
 * Core Audio System for Jurassic Survival Game
 * Handles adaptive music, environmental audio, and dinosaur sound management
 * Based on MetaSounds for procedural audio generation
 */

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Tension        UMETA(DisplayName = "Tension"), 
    Fear           UMETA(DisplayName = "Fear"),
    Terror         UMETA(DisplayName = "Terror"),
    Relief         UMETA(DisplayName = "Relief"),
    Wonder         UMETA(DisplayName = "Wonder")
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    Clearing       UMETA(DisplayName = "Clearing"),
    RiverBank      UMETA(DisplayName = "River Bank"),
    Cave           UMETA(DisplayName = "Cave"),
    Cliffs         UMETA(DisplayName = "Cliffs"),
    Swamp          UMETA(DisplayName = "Swamp")
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    None           UMETA(DisplayName = "None"),
    Herbivore      UMETA(DisplayName = "Herbivore Nearby"),
    SmallPredator  UMETA(DisplayName = "Small Predator"),
    LargePredator  UMETA(DisplayName = "Large Predator"),
    ApexPredator   UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState CurrentEmotion = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType Environment = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 12.0f; // 0-24 hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerHeartRate = 60.0f; // BPM simulation
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurSound(FVector Location, FString DinosaurSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalEvent(FString EventName, FVector Location);

    // Adaptive music control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToEmotionalState(EEmotionalState TargetState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicIntensity(float Intensity); // 0-1

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentAmbience(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDayAudio(float Hour, float TransitionSpeed = 1.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSound assets for adaptive music
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
    class UMetaSoundSource* AmbientForestMetaSound;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbienceAudioComponent;

    // Sound attenuation settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    class USoundAttenuation* DinosaurSoundAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    class USoundAttenuation* EnvironmentalSoundAttenuation;

private:
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    void CalculateEmotionalTransition(EEmotionalState From, EEmotionalState To, float& IntensityTarget, float& TensionTarget);
};