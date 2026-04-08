#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "AudioSystemCore.generated.h"

/**
 * Core Audio System for Transpersonal Game
 * Manages adaptive music, environmental audio, and emotional state-driven soundscapes
 * Based on the principle that perfect audio is invisible to the player
 */

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Tension        UMETA(DisplayName = "Tension"), 
    Fear           UMETA(DisplayName = "Fear"),
    Panic          UMETA(DisplayName = "Panic"),
    Wonder         UMETA(DisplayName = "Wonder"),
    Discovery      UMETA(DisplayName = "Discovery"),
    Isolation      UMETA(DisplayName = "Isolation")
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    OpenPlains     UMETA(DisplayName = "Open Plains"),
    RiverBank      UMETA(DisplayName = "River Bank"),
    CaveSystem     UMETA(DisplayName = "Cave System"),
    CliffSide      UMETA(DisplayName = "Cliff Side"),
    SwampLand      UMETA(DisplayName = "Swamp Land")
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
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 = safe, 1.0 = maximum danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStealthLevel = 1.0f; // 1.0 = completely hidden, 0.0 = fully exposed

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RainIntensity = 0.0f;
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
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurPresence(float Distance, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalEvent(const FString& EventName, FVector Location);

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopAdaptiveMusic(float FadeTime = 3.0f);

    // Silence management - critical for tension
    UFUNCTION(BlueprintCallable, Category = "Silence System")
    void EnterSuspiciousSilence(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Silence System")
    void BreakSilence(const FString& BreakingSound);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSound assets for adaptive music
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TObjectPtr<UMetaSoundSource> AmbientEnvironmentMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TObjectPtr<UMetaSoundSource> TensionLayersMetaSound;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> TensionAudioComponent;

private:
    void UpdateMusicLayers();
    void UpdateAmbientLayers();
    void CalculateEmotionalTransition(EEmotionalState TargetState);
    
    float CurrentTensionLevel = 0.0f;
    float TargetTensionLevel = 0.0f;
    bool bIsSilenceActive = false;
    FTimerHandle SilenceTimerHandle;
};