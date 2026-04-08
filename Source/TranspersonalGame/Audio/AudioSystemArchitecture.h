#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System Architecture for Jurassic Survival Game
 * 
 * This system implements adaptive audio that responds to:
 * - Player emotional state (fear, tension, relief)
 * - Environmental conditions (weather, time of day, biome)
 * - Dinosaur proximity and behavior states
 * - Gameplay context (exploration, hiding, crafting, combat)
 * 
 * Design Philosophy: Audio the player doesn't consciously notice but emotionally feels
 */

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    // Base ambient layers
    Environmental_Base      UMETA(DisplayName = "Environmental Base"),
    Weather_Layer          UMETA(DisplayName = "Weather Layer"),
    TimeOfDay_Layer        UMETA(DisplayName = "Time of Day Layer"),
    
    // Dynamic tension layers
    Tension_Subtle         UMETA(DisplayName = "Subtle Tension"),
    Tension_Medium         UMETA(DisplayName = "Medium Tension"),
    Tension_High           UMETA(DisplayName = "High Tension"),
    Tension_Extreme        UMETA(DisplayName = "Extreme Tension"),
    
    // Creature presence layers
    Herbivore_Distant      UMETA(DisplayName = "Herbivore Distant"),
    Herbivore_Close        UMETA(DisplayName = "Herbivore Close"),
    Predator_Stalking      UMETA(DisplayName = "Predator Stalking"),
    Predator_Hunting       UMETA(DisplayName = "Predator Hunting"),
    
    // Safety and relief
    Base_Safety            UMETA(DisplayName = "Base Safety"),
    Temporary_Relief       UMETA(DisplayName = "Temporary Relief")
};

UENUM(BlueprintType)
enum class ETensionState : uint8
{
    Safe               UMETA(DisplayName = "Safe"),
    Uneasy             UMETA(DisplayName = "Uneasy"),
    Tense              UMETA(DisplayName = "Tense"),
    Danger             UMETA(DisplayName = "Danger"),
    Terror             UMETA(DisplayName = "Terror")
};

UENUM(BlueprintType)
enum class EEnvironmentalContext : uint8
{
    Dense_Forest       UMETA(DisplayName = "Dense Forest"),
    Open_Plains        UMETA(DisplayName = "Open Plains"),
    Rocky_Cliffs       UMETA(DisplayName = "Rocky Cliffs"),
    Swampland          UMETA(DisplayName = "Swampland"),
    Cave_System        UMETA(DisplayName = "Cave System"),
    Riverside          UMETA(DisplayName = "Riverside"),
    Player_Base        UMETA(DisplayName = "Player Base")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETensionState CurrentTension = ETensionState::Safe;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentalContext Environment = EEnvironmentalContext::Dense_Forest;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 12.0f; // 0-24 hours
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0-1
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NearestPredatorDistance = 10000.0f; // In Unreal Units
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NearestHerbivoreDistance = 10000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerIsHidden = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerIsMoving = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerHeartRate = 60.0f; // BPM - affects subtle audio filtering
};

/**
 * Master Audio Controller
 * Manages all adaptive audio layers and responds to game state changes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemController : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemController();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();
    
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTensionLevel(ETensionState NewTension, float TransitionTime = 2.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerAudioEvent(const FString& EventName, FVector Location = FVector::ZeroVector);

protected:
    // MetaSound sources for each layer
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EAudioLayer, UMetaSoundSource*> AudioLayers;
    
    // Audio components for runtime playback
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudioLayer, UAudioComponent*> LayerComponents;
    
    // Current audio state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;
    
    // Transition management
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float DefaultTransitionTime = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Settings")
    float MaxLayerVolume = 0.8f;

private:
    void UpdateEnvironmentalLayers();
    void UpdateTensionLayers();
    void UpdateCreatureLayers();
    void CrossfadeToLayer(EAudioLayer Layer, float TargetVolume, float TransitionTime);
    
    // Subtle audio manipulation based on player state
    void ApplyHeartRateFiltering();
    void ApplyBreathingModulation();
};

/**
 * Dinosaur Audio Behavior Component
 * Handles individual dinosaur vocalizations and movement sounds
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayVocalization(const FString& VocalizationType);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateMovementAudio(float MovementSpeed, bool bIsRunning);
    
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetBehaviorState(const FString& BehaviorState);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<FString, USoundCue*> VocalizationSounds;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    USoundCue* FootstepSounds;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    USoundCue* BreathingSounds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* VocalizationComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MovementComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* BreathingComponent;

private:
    FString CurrentBehaviorState;
    float LastVocalizationTime;
    float VocalizationCooldown = 5.0f;
};