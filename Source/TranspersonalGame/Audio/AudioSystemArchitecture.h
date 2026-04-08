#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System for Jurassic Survival Game
 * Manages adaptive music, environmental audio, and creature behavior sounds
 * Built on UE5 MetaSounds for maximum flexibility and performance
 */

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,           // Safe exploration, building
    Tension = 1,        // Something is nearby but not immediate threat
    Danger = 2,         // Active threat, predator spotted
    Terror = 3,         // Being hunted, immediate life threat
    Wonder = 4,         // Discovery of new species or area
    Isolation = 5       // Complete silence, unnerving emptiness
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains = 1,
    RiverBanks = 2,
    CaveSystem = 3,
    Swampland = 4,
    RockyOutcrops = 5
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle = 0,
    Feeding = 1,
    Hunting = 2,
    Territorial = 3,
    Mating = 4,
    Fleeing = 5,
    Sleeping = 6
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMetaSoundSource* MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping = true;

    FAudioLayerConfig()
    {
        MetaSoundAsset = nullptr;
        BaseVolume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bIsLooping = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core system initialization
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    // Emotional state management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    // Environment audio control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime = 5.0f);

    // Dinosaur audio integration
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurAudio(class ADinosaurCharacter* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateDinosaurBehaviorAudio(class ADinosaurCharacter* Dinosaur, EDinosaurBehaviorState BehaviorState);

    // Proximity and threat detection
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateThreatProximity(float DistanceToNearestThreat, bool bIsBeingHunted = false);

    // Silence management (critical for tension)
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerSuspiciousSilence(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void BreakSilence(bool bWithThreat = true);

private:
    // Current audio state
    UPROPERTY()
    EEmotionalState CurrentEmotionalState;

    UPROPERTY()
    EEnvironmentType CurrentEnvironment;

    // Audio layer management
    UPROPERTY()
    TMap<EEmotionalState, FAudioLayerConfig> EmotionalStateLayers;

    UPROPERTY()
    TMap<EEnvironmentType, FAudioLayerConfig> EnvironmentLayers;

    // Active audio components
    UPROPERTY()
    UAudioComponent* MasterMusicComponent;

    UPROPERTY()
    UAudioComponent* EnvironmentComponent;

    UPROPERTY()
    UAudioComponent* TensionComponent;

    UPROPERTY()
    TArray<UAudioComponent*> DinosaurAudioComponents;

    // Silence system
    UPROPERTY()
    bool bInSuspiciousSilence;

    UPROPERTY()
    float SilenceTimer;

    UPROPERTY()
    FTimerHandle SilenceTimerHandle;

    // Internal methods
    void TransitionToEmotionalState(EEmotionalState TargetState, float TransitionTime);
    void UpdateMusicLayers();
    void HandleSilenceTimeout();
    void SpawnDinosaurAudioComponent(class ADinosaurCharacter* Dinosaur);
};