#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioEmotionalState : uint8
{
    Calm = 0,
    Tension = 1,
    Danger = 2,
    Terror = 3,
    Relief = 4,
    Wonder = 5
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains = 1,
    RiverArea = 2,
    CaveSystem = 3,
    DinosaurNest = 4,
    PlayerBase = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEmotionalState EmotionalState = EAudioEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TensionLevel = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProximityToDanger = 0.0f; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNightTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0.0 to 1.0
};

/**
 * Core Audio System for Jurassic Survival Game
 * Manages adaptive music, environmental audio, and emotional state transitions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core System Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToEmotionalState(EAudioEmotionalState NewState, float TransitionTime = 2.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetEnvironmentType(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAmbientLayers(float DeltaTime);

    // Dinosaur Audio Events
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurCall(const FVector& Location, const FString& DinosaurType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurMovement(const FVector& Location, const FString& DinosaurType, float Speed);

    // Player Audio Events
    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void PlayPlayerHeartbeat(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Player Audio")
    void PlayPlayerBreathing(float Intensity);

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void StopAdaptiveMusic(float FadeTime = 3.0f);

protected:
    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSound Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> EnvironmentalAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurAudioMetaSound;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> PlayerAudioComponent;

    // State Management
    UPROPERTY(BlueprintReadOnly, Category = "State Management")
    float StateTransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State Management")
    EAudioEmotionalState TargetEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "State Management")
    float TransitionDuration = 2.0f;

private:
    void UpdateMetaSoundParameters();
    void CalculateTensionLevel();
    void ProcessEnvironmentalLayers();
    void HandleEmotionalStateTransition(float DeltaTime);
};