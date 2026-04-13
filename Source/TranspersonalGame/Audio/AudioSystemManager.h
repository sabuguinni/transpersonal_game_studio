#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient     UMETA(DisplayName = "Ambient Layer"),
    Melodic     UMETA(DisplayName = "Melodic Layer"),
    Rhythmic    UMETA(DisplayName = "Rhythmic Layer"),
    Spiritual   UMETA(DisplayName = "Spiritual Layer")
};

UENUM(BlueprintType)
enum class EAudio_EmotionalState : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Contemplative   UMETA(DisplayName = "Contemplative"),
    Mystical        UMETA(DisplayName = "Mystical"),
    Transformative  UMETA(DisplayName = "Transformative"),
    Awakening       UMETA(DisplayName = "Awakening")
};

USTRUCT(BlueprintType)
struct FAudio_MusicConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float MelodicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float RhythmicVolume = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float SpiritualVolume = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_EmotionalState CurrentEmotionalState = EAudio_EmotionalState::Peaceful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float TransitionDuration = 3.0f;
};

USTRUCT(BlueprintType)
struct FAudio_SpatialConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float MaxAudibleDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float AttenuationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    bool bUse3DAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Audio")
    float DopplerFactor = 1.0f;
};

/**
 * Audio System Manager - Handles adaptive music, spatial audio, and consciousness-based sound design
 * Implements Walter Murch's philosophy: the sound that doesn't exist is often more powerful than the sound that does
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EAudio_EmotionalState NewState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicLayerVolume(EAudio_MusicLayer Layer, float Volume, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayConsciousnessShiftSound(FVector Location = FVector::ZeroVector);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSpatialAudioSource(AActor* SourceActor, USoundCue* SoundCue, const FAudio_SpatialConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateListenerPosition(FVector NewPosition, FRotator NewRotation);

    // Ambient System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiomeAmbience(const FString& BiomeName, float FadeTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void AddAmbienceLayer(const FString& LayerName, USoundCue* SoundCue, float Volume = 0.5f);

    // Voice & Narration
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarration(USoundWave* NarrationSound, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetNarrationVolume(float Volume);

    // Debug & Testing
    UFUNCTION(BlueprintCallable, Category = "Audio System", CallInEditor)
    void TestAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void LogCurrentAudioState();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAudio_MusicConfiguration MusicConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAudio_SpatialConfiguration DefaultSpatialConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> MusicLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> AmbienceLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* NarrationComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SFXComponent;

private:
    void InitializeMusicLayers();
    void InitializeAmbienceLayers();
    void UpdateMusicBasedOnEmotionalState();
    void CrossfadeToNewState(EAudio_EmotionalState NewState, float Duration);

    FTimerHandle MusicTransitionTimer;
    EAudio_EmotionalState PreviousEmotionalState;
    bool bIsTransitioning;
};