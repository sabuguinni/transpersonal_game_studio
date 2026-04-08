#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,           // Exploração pacífica
    Tension,        // Dinossauros próximos mas não hostis
    Danger,         // Predadores detectados
    Chase,          // Sendo perseguido
    Hide,           // Escondido/stealth
    Wonder,         // Descoberta de algo novo
    Domestication,  // Interagindo com dinossauros domesticáveis
    Survival        // Construção/crafting
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest,
    OpenPlains,
    RiverBank,
    Cave,
    Cliff,
    Swamp,
    ClearingSmall,
    ClearingLarge
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night,
    DeepNight
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
    float ThreatLevel = 0.0f; // 0.0 = seguro, 1.0 = perigo máximo

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExplorationProgress = 0.0f; // 0.0 = início, 1.0 = próximo do fim

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToClosestPredator = 10000.0f;
};

/**
 * Sistema central de áudio que gere música adaptativa, efeitos sonoros e ambientes
 * Baseado na convicção de que o áudio perfeito é invisível ao jogador
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Inicialização do sistema
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    // Actualização do estado emocional
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    // Controlo de música adaptativa
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    // Efeitos sonoros contextuais
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayContextualSFX(const FString& EventName, FVector Location = FVector::ZeroVector);

    // Sistema de ambientes
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateEnvironmentalAudio(EEnvironmentType Environment, ETimeOfDay TimeOfDay);

    // Dinossauros - sons únicos por indivíduo
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurAudio(int32 DinosaurID, const FString& Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(int32 DinosaurID, const FString& SoundType, float EmotionalIntensity = 0.5f);

    // Controlo de volume master por categoria
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(const FString& Category, float Volume);

protected:
    // Estado actual do sistema
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // MetaSounds para música adaptativa
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    // Componente de áudio principal
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    // Mapeamento de sons por espécie de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur Audio")
    TMap<FString, TObjectPtr<UMetaSoundSource>> DinosaurSoundsBySpecies;

    // Tracking de dinossauros individuais
    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Audio")
    TMap<int32, FAudioStateData> DinosaurAudioStates;

private:
    // Transições suaves entre estados
    void BlendToNewMusicState(EEmotionalState NewState, float BlendTime);
    
    // Cálculo de parâmetros procedurais
    float CalculateTensionLevel() const;
    float CalculateEnvironmentalDensity() const;
    
    // Gestão de memória de áudio
    void OptimizeAudioMemory();
    
    // Timer para actualizações automáticas
    FTimerHandle AudioUpdateTimer;
    void PerformPeriodicAudioUpdate();
};