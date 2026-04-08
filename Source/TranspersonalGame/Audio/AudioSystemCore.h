#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,
    Tension,
    Danger,
    Terror,
    Relief,
    Discovery,
    Domestication
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains,
    RiverBanks,
    Caves,
    PlayerBase,
    DinosaurTerritory
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
struct FAudioStateParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0-1, influences all audio

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStealthLevel = 1.0f; // 0-1, affects ambient volume

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInPlayerBase = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToNearestPredator = 1000.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UAudioSystemCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core state management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateParameters& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime = 3.0f);

    // Music system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic(float FadeOutTime = 5.0f);

    // Ambient system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbientLayers();

    // Event-driven audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurEncounter(class ADinosaur* Dinosaur, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDomesticationMoment(class ADinosaur* Dinosaur, bool bSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDiscoveryMoment(const FString& DiscoveryType);

protected:
    UPROPERTY()
    FAudioStateParameters CurrentAudioState;

    UPROPERTY()
    FAudioStateParameters TargetAudioState;

    // Core audio components
    UPROPERTY()
    class UAudioComponent* MusicComponent;

    UPROPERTY()
    class UAudioComponent* AmbientComponent;

    UPROPERTY()
    TArray<class UAudioComponent*> LayeredAmbientComponents;

    // MetaSound assets
    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    class UMetaSoundSource* AmbientLayersMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TMap<EEnvironmentType, class UMetaSoundSource*> EnvironmentMetaSounds;

    // Audio parameter names for MetaSounds
    static const FName PARAM_EMOTIONAL_STATE;
    static const FName PARAM_THREAT_LEVEL;
    static const FName PARAM_TIME_OF_DAY;
    static const FName PARAM_ENVIRONMENT_TYPE;
    static const FName PARAM_STEALTH_LEVEL;
    static const FName PARAM_DINOSAUR_PROXIMITY;

private:
    void InitializeAudioComponents();
    void UpdateMetaSoundParameters();
    void HandleStateTransition(float DeltaTime);

    FTimerHandle StateTransitionTimer;
    float TransitionProgress = 0.0f;
    float TransitionDuration = 2.0f;
};