#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Safe           UMETA(DisplayName = "Safe"),
    Cautious       UMETA(DisplayName = "Cautious"), 
    Tense          UMETA(DisplayName = "Tense"),
    Danger         UMETA(DisplayName = "Danger"),
    Terror         UMETA(DisplayName = "Terror"),
    Relief         UMETA(DisplayName = "Relief")
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    OpenPlain      UMETA(DisplayName = "Open Plain"),
    Riverside      UMETA(DisplayName = "Riverside"),
    Cave           UMETA(DisplayName = "Cave"),
    Clearing       UMETA(DisplayName = "Clearing"),
    Swamp          UMETA(DisplayName = "Swamp")
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

USTRUCT(BlueprintType)
struct FAudioContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 = completely safe, 1.0 = maximum threat

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerHeartRate = 60.0f; // Simulated player stress level

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHiding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRecentlyEscapedDanger = false;
};

/**
 * Core audio system that manages adaptive music, ambient soundscapes, 
 * and emotional state-driven audio for the Jurassic survival game.
 * 
 * This system ensures that audio is the sense the player doesn't know
 * they're using - constantly informing about danger, safety, and emotional state.
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioSystemCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Main interface for updating audio context
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioContext(const FAudioContext& NewContext);

    // Emotional state management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(float NewThreatLevel, float TransitionTime = 1.0f);

    // Environment and time management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(ETimeOfDay NewTimeOfDay, float TransitionTime = 10.0f);

    // Immediate audio responses
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurPresence(FVector Location, float IntensityLevel = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerSilenceBreak(FVector Location, float IntensityLevel = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerPlayerHiding();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerPlayerExitHiding();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio System")
    FAudioContext GetCurrentAudioContext() const { return CurrentContext; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentContext.EmotionalState; }

private:
    // Current audio context
    UPROPERTY()
    FAudioContext CurrentContext;

    // Audio components for layered soundscape
    UPROPERTY()
    UAudioComponent* MusicComponent;

    UPROPERTY()
    UAudioComponent* AmbientComponent;

    UPROPERTY()
    UAudioComponent* TensionComponent;

    UPROPERTY()
    UAudioComponent* BreathingComponent;

    // MetaSound assets for adaptive audio
    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* AmbientSoundscapeMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* TensionLayerMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* HeartbeatMetaSound;

    // Transition management
    void UpdateMusicLayer();
    void UpdateAmbientLayer();
    void UpdateTensionLayer();
    void UpdateBreathingLayer();

    // Utility functions
    float CalculateOverallIntensity() const;
    float GetEnvironmentReverb() const;
    float GetTimeOfDayBrightness() const;
};