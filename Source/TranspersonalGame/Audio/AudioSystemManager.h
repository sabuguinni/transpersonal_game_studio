#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Exploration,
    Tension,
    Danger,
    Combat,
    Safety,
    Discovery,
    Domestication
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest,
    OpenPlains,
    RiverBanks,
    CaveSystem,
    PlayerBase,
    DinosaurNest
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TensionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProximityThreat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType CurrentEnvironment = EEnvironmentType::DenseForest;
};

/**
 * Sistema central de gerenciamento de áudio adaptativo
 * Controla música dinâmica, efeitos ambientes e espacialização 3D
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Estado do Sistema
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& StateData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioState(EAudioState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    EAudioState GetCurrentAudioState() const { return CurrentAudioState; }

    // Música Adaptativa
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity);

    // Ambiente Sonoro
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbientAudio(EEnvironmentType Environment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurAmbient(FVector Location, float Intensity);

    // Sistema de Proximidade
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurProximity(AActor* Dinosaur, float Distance, bool bIsPredator);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterDinosaurProximity(AActor* Dinosaur);

protected:
    // Estado Atual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EAudioState CurrentAudioState = EAudioState::Exploration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentStateData;

    // Componentes de Áudio
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* TensionAudioComponent;

    // Assets de Áudio
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AmbientForestMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> TensionMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<USoundMix> ExplorationSoundMix;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<USoundMix> DangerSoundMix;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<USoundMix> CombatSoundMix;

    // Controle de Transições
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Control")
    float StateTransitionTime = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Control")
    float CurrentMusicIntensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Control")
    float TargetMusicIntensity = 0.0f;

private:
    // Métodos Internos
    void TransitionToState(EAudioState NewState);
    void UpdateMusicParameters();
    void UpdateAmbientParameters();
    void CalculateTensionLevel();
    
    // Timers
    FTimerHandle MusicUpdateTimer;
    FTimerHandle StateUpdateTimer;

    // Dados de Proximidade
    TMap<AActor*, float> DinosaurProximityMap;
    TMap<AActor*, bool> DinosaurThreatMap;
};