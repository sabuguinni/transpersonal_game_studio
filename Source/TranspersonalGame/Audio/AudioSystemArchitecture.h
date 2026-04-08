#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Components/AudioComponent.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System Architecture for Jurassic Survival Game
 * Implements adaptive audio system with emotional state management
 */

UENUM(BlueprintType)
enum class EAudioEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Tension        UMETA(DisplayName = "Tension"), 
    Fear           UMETA(DisplayName = "Fear"),
    Panic          UMETA(DisplayName = "Panic"),
    Wonder         UMETA(DisplayName = "Wonder"),
    Discovery      UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EAudioEnvironmentType : uint8
{
    DenseForest    UMETA(DisplayName = "Dense Forest"),
    OpenPlains     UMETA(DisplayName = "Open Plains"),
    RiverBank      UMETA(DisplayName = "River Bank"),
    Cave           UMETA(DisplayName = "Cave"),
    Clearing       UMETA(DisplayName = "Clearing"),
    Swamp          UMETA(DisplayName = "Swamp")
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    None           UMETA(DisplayName = "No Threat"),
    Herbivore      UMETA(DisplayName = "Herbivore Nearby"),
    SmallPredator  UMETA(DisplayName = "Small Predator"),
    LargePredator  UMETA(DisplayName = "Large Predator"),
    ApexPredator   UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FAudioStateParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudioEmotionalState EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EAudioEnvironmentType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    EDinosaurThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IsolationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay; // 0.0 = Dawn, 0.25 = Day, 0.5 = Dusk, 0.75 = Night

    FAudioStateParameters()
    {
        EmotionalState = EAudioEmotionalState::Calm;
        EnvironmentType = EAudioEnvironmentType::DenseForest;
        ThreatLevel = EDinosaurThreatLevel::None;
        StressLevel = 0.0f;
        IsolationLevel = 0.0f;
        TimeOfDay = 0.25f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateParameters& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToEmotionalState(EAudioEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudioEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateThreatLevel(EDinosaurThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerDinosaurEvent(const FString& DinosaurType, const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic(float FadeOutTime = 3.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioStateParameters CurrentAudioState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> StingerAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Classes")
    TObjectPtr<USoundClass> MasterSoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Classes")
    TObjectPtr<USoundClass> MusicSoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Classes")
    TObjectPtr<USoundClass> SFXSoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Classes")
    TObjectPtr<USoundClass> AmbienceSoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Classes")
    TObjectPtr<USoundClass> VoiceSoundClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Mixes")
    TObjectPtr<USoundMix> DefaultSoundMix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Mixes")
    TObjectPtr<USoundMix> TensionSoundMix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Mixes")
    TObjectPtr<USoundMix> FearSoundMix;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound Mixes")
    TObjectPtr<USoundMix> PanicSoundMix;

private:
    void UpdateMusicLayer();
    void UpdateAmbienceLayer();
    void ApplySoundMixForState();
    void ProcessAudioTransition(float DeltaTime);

    float TransitionTimer;
    float TransitionDuration;
    EAudioEmotionalState TargetEmotionalState;
    bool bIsTransitioning;
};