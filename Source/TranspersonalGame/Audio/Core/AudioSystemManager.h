#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Calm,
    Tense,
    Danger,
    Combat,
    Discovery,
    Domestication,
    Exploration
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest,
    OpenPlains,
    RiverBanks,
    Caves,
    Cliffs,
    Swamplands
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core MetaSound Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(EAudioState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType Environment);

    // Dynamic Music System
    UFUNCTION(BlueprintCallable, Category = "Music")
    void TriggerMusicTransition(EAudioState TargetState, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicIntensity(float Intensity); // 0.0 = minimal, 1.0 = maximum

    // Ambient Audio
    UFUNCTION(BlueprintCallable, Category = "Ambient")
    void UpdateAmbientLayers(EEnvironmentType Environment, float TimeOfDay, float WeatherIntensity);

    // Dinosaur Audio
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void PlayDinosaurCall(class ADinosaurCharacter* Dinosaur, FString CallType);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Audio")
    void UpdateDinosaurProximityAudio(TArray<class ADinosaurCharacter*> NearbyDinosaurs);

protected:
    // MetaSound Sources
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> MasterMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AmbientLayersMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurAudioMetaSound;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudioState CurrentAudioState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEnvironmentType CurrentEnvironment;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentMusicIntensity;

private:
    void InitializeMetaSounds();
    void SetupAudioComponents();
    void UpdateMetaSoundParameters();
};