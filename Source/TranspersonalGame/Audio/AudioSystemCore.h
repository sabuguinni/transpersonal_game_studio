#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "GameFramework/GameStateBase.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Tense          UMETA(DisplayName = "Tense"), 
    Danger         UMETA(DisplayName = "Danger"),
    Terror         UMETA(DisplayName = "Terror"),
    Discovery      UMETA(DisplayName = "Discovery"),
    Bonding        UMETA(DisplayName = "Bonding")
};

UENUM(BlueprintType)
enum class EDinosaurSize : uint8
{
    Tiny           UMETA(DisplayName = "Tiny (< 1m)"),
    Small          UMETA(DisplayName = "Small (1-3m)"),
    Medium         UMETA(DisplayName = "Medium (3-8m)"),
    Large          UMETA(DisplayName = "Large (8-15m)"),
    Massive        UMETA(DisplayName = "Massive (> 15m)")
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Sleeping       UMETA(DisplayName = "Sleeping"),
    Grazing        UMETA(DisplayName = "Grazing"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Territorial    UMETA(DisplayName = "Territorial"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Socializing    UMETA(DisplayName = "Socializing"),
    Domesticated   UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> MetaSoundAsset;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurAudioSignature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurSize Size = EDinosaurSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BasePitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttenuationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EDinosaurBehavior, TObjectPtr<UMetaSoundSource>> BehaviorSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> FootstepSound;
};

/**
 * Core Audio System for Jurassic Survival Game
 * Manages adaptive music, environmental audio, and dinosaur soundscapes
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
    void UpdateEmotionalState(EEmotionalState NewState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float TimeNormalized); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

    // Dinosaur Audio Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void RegisterDinosaur(AActor* DinosaurActor, const FDinosaurAudioSignature& AudioSignature);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateDinosaurBehavior(AActor* DinosaurActor, EDinosaurBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void SetDinosaurDomestication(AActor* DinosaurActor, float DomesticationLevel); // 0.0 = wild, 1.0 = fully tamed

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerEnvironmentalEvent(const FString& EventName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetPlayerStealthLevel(float StealthLevel); // 0.0 = loud movement, 1.0 = silent

protected:
    // Audio Layer Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TMap<FString, FAudioLayerConfig> AudioLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional States")
    TMap<EEmotionalState, TObjectPtr<UMetaSoundSource>> EmotionalMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TMap<FString, TObjectPtr<UMetaSoundSource>> EnvironmentalSounds;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEmotionalState CurrentEmotionalState = EEmotionalState::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentWeatherIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float PlayerStealthLevel = 0.5f;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> AmbienceComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAudioComponent> WeatherComponent;

    // Registered Dinosaurs
    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, FDinosaurAudioSignature> RegisteredDinosaurs;

private:
    void UpdateMusicLayer();
    void UpdateAmbienceLayer();
    void UpdateWeatherLayer();
    void ProcessDinosaurAudio();
    
    float CalculateTensionLevel() const;
    void CrossfadeToNewMusic(UMetaSoundSource* NewMusic, float FadeTime);
};