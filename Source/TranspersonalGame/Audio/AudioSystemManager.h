#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Calm,
    Tension,
    Danger,
    Panic,
    Stealth,
    Discovery,
    Safety
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    Forest,
    Plains,
    Swamp,
    Cave,
    River,
    Clearing
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioState CurrentState = EAudioState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType Environment = EEnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::None;
};

UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Estado do sistema de áudio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioState(EAudioState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioSystem(float DeltaTime);

    // Música adaptativa
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic();

    // Efeitos sonoros contextuais
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurType, const FVector& Location, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentSound(const FString& SoundType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayPlayerActionSound(const FString& ActionType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System")
    FAudioStateData CurrentAudioState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio System")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    // MetaSounds para diferentes estados
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TMap<EAudioState, TObjectPtr<UMetaSoundSource>> StateMetaSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TMap<EEnvironmentType, TObjectPtr<UMetaSoundSource>> EnvironmentMetaSounds;

    // Parâmetros de controle
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    float AmbienceVolume = 0.8f;

private:
    void InitializeAudioComponents();
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    float CalculateStateBlend(EAudioState TargetState, float DeltaTime);
};