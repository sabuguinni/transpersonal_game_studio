#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "AudioSystemCore.generated.h"

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
    Forest,
    River,
    Cave,
    Clearing,
    DinosaurTerritory,
    PlayerBase
};

USTRUCT(BlueprintType)
struct FAudioStateConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MusicVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbienceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowPassCutoff = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReverbWetness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionTime = 2.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UAudioSystemCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Estado principal do sistema
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioState(EAudioState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTensionLevel(float TensionLevel); // 0.0 = calmo, 1.0 = máximo perigo

    // Controle de música adaptativa
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAdaptiveMusic(class UMetaSoundSource* MusicMetaSound);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic(float FadeTime = 2.0f);

    // Controle de ambiente sonoro
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbienceForLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(class ADinosaurBase* Dinosaur, FName SoundType);

    // Sistema de oclusão e reverb dinâmico
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAcousticEnvironment(FVector ListenerLocation);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    TMap<EAudioState, FAudioStateConfig> AudioStateConfigs;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    class USoundClass* MasterSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    class USoundClass* MusicSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    class USoundClass* AmbienceSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    class USoundClass* SFXSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
    class USoundMix* DynamicSoundMix;

private:
    EAudioState CurrentAudioState;
    EEnvironmentType CurrentEnvironment;
    float CurrentTensionLevel;

    UPROPERTY()
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    class UAudioComponent* AmbienceAudioComponent;

    void ApplyAudioStateConfig(const FAudioStateConfig& Config);
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
};