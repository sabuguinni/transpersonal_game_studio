#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

class UMetaSoundSource;
class USoundAttenuation;
class UAudioComponent;

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Tension,
    Danger,
    Terror,
    Relief,
    Wonder,
    Domestication
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    Forest,
    Plains,
    Swamp,
    Mountains,
    Cave,
    River,
    Clearing
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Feeding,
    Hunting,
    Sleeping,
    Alert,
    Aggressive,
    Fleeing,
    Domesticated
};

USTRUCT(BlueprintType)
struct FAudioLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMetaSoundSource* MetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    UAudioComponent* AudioComponent = nullptr;
};

USTRUCT(BlueprintType)
struct FDinosaurAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DinosaurID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EDinosaurBehaviorState, UMetaSoundSource*> BehaviorSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoicePitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumeVariation = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundAttenuation* AttenuationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DomesticationProgressAudio = 0.0f; // 0.0 = wild, 1.0 = fully domesticated
};

/**
 * Sistema central de áudio que gere música adaptativa, ambientes sonoros e comportamento de dinossauros
 * Baseado nos princípios de Walter Murch: o som que não existe é muitas vezes mais poderoso
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Música Adaptativa
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment, float TransitionTime = 2.0f);

    // Sistema de Dinossauros
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaur(const FString& DinosaurID, const FDinosaurAudioProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateDinosaurBehavior(const FString& DinosaurID, EDinosaurBehaviorState NewBehavior, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateDinosaurDomestication(const FString& DinosaurID, float DomesticationLevel);

    // Ambiente Sonoro
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float TimeNormalized); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

    // Sistema de Tensão
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void AddTensionEvent(FVector Location, float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSilenceMode(bool bEnabled, float FadeTime = 1.0f);

    // Eventos de Gameplay
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerHidden(bool bIsHidden);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerDetected(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnCraftingActivity(bool bStarted);

private:
    // Estado atual do sistema
    UPROPERTY()
    EEmotionalState CurrentEmotionalState = EEmotionalState::Calm;

    UPROPERTY()
    EEnvironmentType CurrentEnvironment = EEnvironmentType::Forest;

    UPROPERTY()
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY()
    float CurrentWeatherIntensity = 0.0f;

    UPROPERTY()
    bool bInSilenceMode = false;

    // Layers de áudio
    UPROPERTY()
    TMap<EEmotionalState, FAudioLayer> EmotionalLayers;

    UPROPERTY()
    TMap<EEnvironmentType, FAudioLayer> EnvironmentLayers;

    UPROPERTY()
    FAudioLayer WeatherLayer;

    UPROPERTY()
    FAudioLayer TensionLayer;

    // Dinossauros registados
    UPROPERTY()
    TMap<FString, FDinosaurAudioProfile> DinosaurProfiles;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveDinosaurAudio;

    // Métodos internos
    void UpdateMusicLayers();
    void UpdateEnvironmentAudio();
    void ProcessTensionEvents();
    void UpdateDinosaurAudioParameters(const FString& DinosaurID);

    // Timers e eventos
    FTimerHandle TensionUpdateTimer;
    FTimerHandle EnvironmentUpdateTimer;

    // Configurações
    UPROPERTY()
    USoundClass* MasterSoundClass;

    UPROPERTY()
    USoundClass* MusicSoundClass;

    UPROPERTY()
    USoundClass* EnvironmentSoundClass;

    UPROPERTY()
    USoundClass* DinosaurSoundClass;

    UPROPERTY()
    USoundMix* DynamicSoundMix;
};