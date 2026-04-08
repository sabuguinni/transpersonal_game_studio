#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "AudioArchitecture.generated.h"

/**
 * Sistema central de áudio adaptativo para o jogo de dinossauros
 * Controla música dinâmica, ambientes sonoros e efeitos contextuais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioManager();

    // Estados emocionais principais
    UENUM(BlueprintType)
    enum class EEmotionalState : uint8
    {
        Calm_Wonder,        // Exploração pacífica, admiração
        Tension_Building,   // Algo está errado, tensão crescente
        Immediate_Danger,   // Predador próximo, fuga necessária
        Relief_Safety,      // Chegada a área segura
        Domestication,      // Interação com herbívoros amigáveis
        Discovery          // Encontrar pistas sobre a gema
    };

    // Tipos de ambiente sonoro
    UENUM(BlueprintType)
    enum class EAmbientType : uint8
    {
        Dense_Forest,       // Floresta densa, sons abafados
        Open_Plains,        // Planícies, sons distantes
        Water_Edge,         // Próximo a rios/lagos
        Cave_System,        // Cavernas, eco natural
        Danger_Zone        // Área com predadores ativos
    };

    // Sistema de música adaptativa
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TMap<EEmotionalState, UMetaSoundSource*> EmotionalTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    TMap<EAmbientType, UMetaSoundSource*> AmbientSystems;

    // Controle de estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAmbientType CurrentAmbientType;

    // Componentes de áudio principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* DinosaurComponent;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAmbientEnvironment(EAmbientType NewAmbient, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayDinosaurSound(class ADinosaurCharacter* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TriggerStingerSound(const FString& StingerType);

protected:
    virtual void BeginPlay() override;

private:
    // Sistema de transição suave
    FTimerHandle MusicTransitionTimer;
    FTimerHandle AmbientTransitionTimer;
    
    void ExecuteMusicTransition(UMetaSoundSource* NewTrack);
    void ExecuteAmbientTransition(UMetaSoundSource* NewAmbient);
};

/**
 * Volume de áudio que detecta a presença de dinossauros
 * Ajusta automaticamente o estado emocional baseado na proximidade de predadores
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurAudioVolume : public AAudioGameplayVolume
{
    GENERATED_BODY()

public:
    ADinosaurAudioVolume();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Detection")
    float PredatorDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Detection")
    UAudioManager::EEmotionalState TriggerEmotionalState = UAudioManager::EEmotionalState::Tension_Building;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    UAudioManager* AudioManager;

    void CheckForNearbyDinosaurs();
    bool HasPredatorsInRange();
};