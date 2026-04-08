#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Sistema de áudio adaptativo para o jogo jurássico
 * Baseado na convicção de que o som que o jogador não nota conscientemente
 * mas que muda o que sente é o pico da composição interactiva
 */

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,           // Exploração segura
    Tension,            // Suspeita, algo pode estar errado
    Danger,             // Predador próximo mas não detectou jogador
    Hunt,               // Predador está a caçar o jogador
    Relief,             // Escapou de perigo
    Wonder,             // Descoberta de algo novo
    Domestication       // Interacção com dinossauros domesticados
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains,
    RiverBed,
    CaveSystem,
    PlayerBase,
    DinosaurNest,
    AncientRuins
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night,
    DeepNight
};

USTRUCT(BlueprintType)
struct FAudioStateContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType Environment = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 = seguro, 1.0 = perigo máximo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DinosaurActivity = 0.0f; // Densidade de dinossauros na área

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // Intensidade do clima
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAdaptiveAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAdaptiveAudioManager();

    // Sistema de transição musical adaptativa
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateAudioState(const FAudioStateContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void TriggerEmotionalTransition(EEmotionalState NewState, float TransitionTime = 2.0f);

    // Sistema de layers musicais
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetMusicLayer(const FString& LayerName, float Volume, float FadeTime = 1.0f);

    // Sistema de ambient soundscape
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateAmbientSoundscape(EEnvironmentType Environment, ETimeOfDay TimeOfDay);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EEmotionalState, class UMetaSoundSource*> EmotionalMusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EEnvironmentType, class UMetaSoundSource*> EnvironmentAmbients;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<ETimeOfDay, class USoundCue*> TimeOfDayLayers;

    UPROPERTY()
    FAudioStateContext CurrentContext;

    UPROPERTY()
    class UAudioComponent* MusicComponent;

    UPROPERTY()
    class UAudioComponent* AmbientComponent;

    UPROPERTY()
    TArray<class UAudioComponent*> LayerComponents;

private:
    void InitializeAudioComponents();
    void CrossfadeToNewMusic(class UMetaSoundSource* NewMusic, float FadeTime);
    void UpdateAmbientLayers();
};

/**
 * Component para dinossauros individuais
 * Cada dinossauro tem a sua própria assinatura sonora
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurAudioComponent : public UAudioComponent
{
    GENERATED_BODY()

public:
    UDinosaurAudioComponent();

    // Sons específicos do dinossauro
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayVocalization(const FString& VocalizationType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayMovementSound(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayBreathingSound(bool bIsStressed);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur Sounds")
    TMap<FString, class USoundCue*> Vocalizations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur Sounds")
    class UMetaSoundSource* MovementMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur Sounds")
    class USoundCue* BreathingSound;

    // Variações individuais para cada dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Individual Traits")
    float PitchVariation = 1.0f; // Cada dinossauro tem um pitch ligeiramente diferente

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Individual Traits")
    float VolumeVariation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Individual Traits")
    float TimbreVariation = 0.0f; // Modificação de filtros para personalidade única
};