#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioMixerBlueprintLibrary.h"
#include "MetasoundSource.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EAudioEnvironmentType : uint8
{
    Forest_Dense,
    Forest_Open, 
    Swamp,
    Plains,
    Rocky_Canyon,
    Cave_Shallow,
    Cave_Deep,
    Riverside,
    Lake,
    Clifftop
};

UENUM(BlueprintType)
enum class EAudioThreatLevel : uint8
{
    Safe,
    Cautious,
    Danger,
    Panic
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
struct FAudioEnvironmentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EAudioEnvironmentType EnvironmentType = EAudioEnvironmentType::Forest_Dense;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EAudioThreatLevel ThreatLevel = EAudioThreatLevel::Safe;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float PlayerStealthLevel = 0.0f; // 0.0 = loud, 1.0 = silent

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bNearWater = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bInCave = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DinosaurProximity = 0.0f; // 0.0 = none nearby, 1.0 = very close
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateEnvironmentState(const FAudioEnvironmentState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(class ADinosaurBase* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerTensionMusic(EAudioThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Audio System")
    FAudioEnvironmentState CurrentEnvironmentState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    class UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    class UMetaSoundSource* EnvironmentAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EAudioEnvironmentType, class USoundAttenuation*> EnvironmentAttenuationSettings;

private:
    void UpdateAdaptiveMusic();
    void UpdateEnvironmentAmbience();
    void CalculateAudioParameters();

    float CurrentTensionLevel = 0.0f;
    float TargetTensionLevel = 0.0f;
    float TensionTransitionSpeed = 1.0f;
};