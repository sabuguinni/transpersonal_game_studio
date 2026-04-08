#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundAttenuation.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioEmotionalState : uint8
{
    Calm_Exploration,
    Tense_Awareness,
    Fear_Imminent_Danger,
    Terror_Active_Threat,
    Relief_Safety_Found,
    Wonder_Discovery,
    Melancholy_Solitude
};

UENUM(BlueprintType)
enum class EAudioEnvironmentType : uint8
{
    Dense_Forest,
    Open_Plains,
    Rocky_Cliffs,
    Water_Edge,
    Cave_Interior,
    Ancient_Ruins
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEmotionalState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioEmotionalState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UCurveFloat* TransitionCurve;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateEmotionalState(EAudioEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudioEnvironmentType Environment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurProximityAudio(float Distance, float DinosaurSize);

    // MetaSound parameter control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAmbienceLayer(int32 LayerIndex, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerStingerSound(const FString& StingerType);

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioEnvironmentType CurrentEnvironment;

    // MetaSound assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    UMetaSoundSource* EnvironmentAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    UMetaSoundSource* DinosaurProximityMetaSound;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ProximityAudioComponent;

    // State transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transitions")
    TArray<FAudioStateTransition> StateTransitions;

    // Internal functions
    void CreateAudioComponents();
    void ApplyEmotionalStateParameters();
    void HandleStateTransition(EAudioEmotionalState NewState, float Duration);
};