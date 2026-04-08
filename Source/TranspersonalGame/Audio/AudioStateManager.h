#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioStateManager.generated.h"

UENUM(BlueprintType)
enum class EAudioEmotionalState : uint8
{
    Prey        UMETA(DisplayName = "Prey Mode"),
    Observation UMETA(DisplayName = "Observation Mode"),
    Danger      UMETA(DisplayName = "Danger Mode"),
    Domestication UMETA(DisplayName = "Domestication Mode"),
    Exploration UMETA(DisplayName = "Exploration Mode")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe Zone"),
    Awareness   UMETA(DisplayName = "Awareness Zone"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    Critical    UMETA(DisplayName = "Critical Zone")
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RiverBed        UMETA(DisplayName = "River Bed"),
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop"),
    Swampland       UMETA(DisplayName = "Swampland")
};

USTRUCT(BlueprintType)
struct FAudioMixSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float MusicVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float AmbienceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float DinosaurVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float PlayerVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float InteractiveVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Settings")
    float TransitionTime = 2.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioStateChanged, EAudioEmotionalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatLevelChanged, EThreatLevel, NewThreatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeChanged, EBiomeType, NewBiome);

/**
 * Central manager for all audio states and adaptive music system
 * Communicates with AI, weather, and player systems to create dynamic soundscape
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioStateManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioStateManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ThreatAudioComponent;

    // MetaSound Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* AdaptiveMusicController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* ThreatDetectionAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    class UMetaSoundSource* EnvironmentalAmbience;

    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentStressLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TimeOfDay;

    // Mix Settings for Different States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Presets")
    TMap<EAudioEmotionalState, FAudioMixSettings> EmotionalStateMixSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Presets")
    TMap<EThreatLevel, FAudioMixSettings> ThreatLevelMixSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mix Presets")
    TMap<EBiomeType, FAudioMixSettings> BiomeMixSettings;

public:
    // State Change Functions
    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetEmotionalState(EAudioEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetBiome(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetStressLevel(float NewStressLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetTimeOfDay(float NewTimeOfDay);

    // Threat Detection
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void RegisterPredatorProximity(class ADinosaurCharacter* Predator, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void UnregisterPredatorProximity(class ADinosaurCharacter* Predator);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void TriggerPredatorAudio(class ADinosaurCharacter* Predator, FVector Location);

    // Music Control
    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void PlayMusicalStinger(class USoundCue* Stinger);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void CrossfadeToMusicState(FName StateName, float CrossfadeTime = 2.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(float WindIntensity, float RainIntensity, float ThunderProbability);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayEnvironmentalOneShot(class USoundCue* Sound, FVector Location, float VolumeMultiplier = 1.0f);

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnAudioStateChanged OnAudioStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnThreatLevelChanged OnThreatLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnBiomeChanged OnBiomeChanged;

private:
    // Internal Functions
    void UpdateAudioMix();
    void CalculateThreatLevel();
    void UpdateMetaSoundParameters();
    
    // Threat tracking
    TMap<class ADinosaurCharacter*, float> TrackedPredators;
    
    // Audio parameter caching
    float LastMusicIntensity;
    float LastAmbienceLevel;
    float LastThreatLevel;
    
    // Transition timers
    float StateTransitionTimer;
    bool bIsTransitioning;
    FAudioMixSettings TargetMixSettings;
    FAudioMixSettings CurrentMixSettings;
};