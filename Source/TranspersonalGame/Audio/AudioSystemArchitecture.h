#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System Architecture for Jurassic Survival Game
 * Implements Walter Murch's philosophy: "The sound that doesn't exist is often more powerful than the sound that does"
 * 
 * Design Principles:
 * 1. Adaptive music that changes based on threat level and emotional state
 * 2. 3D positional audio with realistic attenuation for prehistoric environment
 * 3. Dynamic ambient soundscapes that respond to time of day and weather
 * 4. Procedural dinosaur vocalizations with individual character variations
 * 5. Silence as a tool for building tension
 */

UENUM(BlueprintType)
enum class EAudioThreatLevel : uint8
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
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop"),
    Cave            UMETA(DisplayName = "Cave"),
    Swampland       UMETA(DisplayName = "Swampland")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Midday      UMETA(DisplayName = "Midday"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioThreatLevel ThreatLevel = EAudioThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType Environment = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStealthLevel = 0.0f; // 0.0 = loud, 1.0 = silent

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerHiding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToNearestPredator = 10000.0f; // In Unreal Units
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EAudioThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurVocalization(FVector Location, class ADinosaur* DinosaurRef);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalEvent(FVector Location, FString EventType);

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Music")
    void TransitionToMusicState(EAudioThreatLevel TargetState, float TransitionTime = 2.0f);

    // Ambient system
    UFUNCTION(BlueprintCallable, Category = "Ambient")
    void UpdateAmbientLayers();

    // Silence system - crucial for building tension
    UFUNCTION(BlueprintCallable, Category = "Tension")
    void TriggerTensionSilence(float Duration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Tension")
    void BuildTensionWithSubtleSounds();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioState CurrentAudioState;

    // MetaSound assets for adaptive music
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    // Ambient soundscape MetaSounds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
    TMap<EEnvironmentType, TObjectPtr<UMetaSoundSource>> EnvironmentAmbientSounds;

    // Dinosaur vocalization library
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaurs")
    TMap<FString, TObjectPtr<USoundCue>> DinosaurVocalizations;

    // Audio components for persistent sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

private:
    // Transition management
    float CurrentMusicTransitionTime;
    EAudioThreatLevel TargetThreatLevel;
    bool bIsTransitioning;

    // Tension system
    FTimerHandle TensionSilenceTimer;
    bool bInTensionSilence;

    // Performance optimization
    float LastAudioUpdateTime;
    const float AudioUpdateInterval = 0.1f; // Update 10 times per second
};