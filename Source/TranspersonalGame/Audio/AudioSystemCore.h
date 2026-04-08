#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Tension        UMETA(DisplayName = "Tension"),
    Fear           UMETA(DisplayName = "Fear"),
    Wonder         UMETA(DisplayName = "Wonder"),
    Danger         UMETA(DisplayName = "Danger"),
    Discovery      UMETA(DisplayName = "Discovery"),
    Stalking       UMETA(DisplayName = "Stalking"),
    Chase          UMETA(DisplayName = "Chase"),
    Safety         UMETA(DisplayName = "Safety"),
    Loneliness     UMETA(DisplayName = "Loneliness")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn           UMETA(DisplayName = "Dawn"),
    Morning        UMETA(DisplayName = "Morning"),
    Midday         UMETA(DisplayName = "Midday"),
    Afternoon      UMETA(DisplayName = "Afternoon"),
    Dusk           UMETA(DisplayName = "Dusk"),
    Night          UMETA(DisplayName = "Night"),
    DeepNight      UMETA(DisplayName = "Deep Night")
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    OpenPlains     UMETA(DisplayName = "Open Plains"),
    RiverBank      UMETA(DisplayName = "River Bank"),
    Caves          UMETA(DisplayName = "Caves"),
    Cliffs         UMETA(DisplayName = "Cliffs"),
    Swampland      UMETA(DisplayName = "Swampland")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBiomeType CurrentBiome = EBiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 = Safe, 1.0 = Maximum Danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExplorationProgress = 0.0f; // 0.0 = Just started, 1.0 = Near end

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNearDinosaurs = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToNearestThreat = 1000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(ETimeOfDay NewTime, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiome(EBiomeType NewBiome, float TransitionTime = 3.0f);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerMusicTransition(EEmotionalState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity); // 0.0 to 1.0

    // Ambient Audio System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbientLayers();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(FVector Location, FString DinosaurType, FString SoundType);

    // Procedural Audio Events
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerFootstepVariation(FVector Location, FString SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalEvent(FVector Location, FString EventType);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void Update3DAudioPosition(FVector ListenerLocation, FRotator ListenerRotation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetOcclusionLevel(float OcclusionAmount);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio System")
    FAudioStateData GetCurrentAudioState() const { return CurrentAudioState; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentAudioState.EmotionalState; }

protected:
    // Core State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData PreviousAudioState;

    // MetaSound References
    UPROPERTY(EditDefaultsOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AmbientLayersMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurAudioMetaSound;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> DynamicAudioComponents;

    // Transition Management
    UPROPERTY(EditDefaultsOnly, Category = "Transitions")
    float DefaultTransitionTime = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Transitions")
    UCurveFloat* EmotionalTransitionCurve;

private:
    void InitializeAudioComponents();
    void ProcessEmotionalTransition(EEmotionalState FromState, EEmotionalState ToState, float TransitionTime);
    void UpdateMetaSoundParameters();
    float CalculateEmotionalIntensity() const;
    void CleanupFinishedAudioComponents();
};