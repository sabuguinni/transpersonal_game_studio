#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTensionStateChanged, float, NewTensionLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurVocalization, class ADinosaur*, Dinosaur, ESoundType, SoundType);

UENUM(BlueprintType)
enum class ETensionState : uint8
{
    Calm = 0,
    Unease = 1,
    Tension = 2,
    Danger = 3,
    Terror = 4
};

UENUM(BlueprintType)
enum class ESoundType : uint8
{
    Breathing = 0,
    Footsteps = 1,
    Vocalization = 2,
    Movement = 3,
    Feeding = 4,
    Territorial = 5,
    Mating = 6,
    Distress = 7
};

UENUM(BlueprintType)
enum class EEnvironmentState : uint8
{
    DayCalm = 0,
    DayActive = 1,
    DuskTension = 2,
    NightDanger = 3,
    DawnRelief = 4,
    Storm = 5,
    Silence = 6
};

/**
 * Central audio system manager for the Jurassic survival game
 * Handles adaptive music, environmental audio, and creature sound systems
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tension System
    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    void UpdateTensionLevel(float NewTensionLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    float GetCurrentTensionLevel() const { return CurrentTensionLevel; }
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    ETensionState GetCurrentTensionState() const { return CurrentTensionState; }

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetEnvironmentState(EEnvironmentState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void UpdateTimeOfDay(float TimeOfDayNormalized);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetWeatherIntensity(float WeatherIntensity);

    // Creature Audio
    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void RegisterDinosaur(class ADinosaur* Dinosaur);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void UnregisterDinosaur(class ADinosaur* Dinosaur);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Creatures")
    void PlayDinosaurSound(class ADinosaur* Dinosaur, ESoundType SoundType, float Volume = 1.0f);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionToMusicState(const FString& StateName, float TransitionTime = 2.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicParameter(const FString& ParameterName, float Value);

    // Silence System (Walter Murch inspired)
    UFUNCTION(BlueprintCallable, Category = "Audio|Silence")
    void TriggerSilenceMoment(float Duration, float FadeTime = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Audio|Silence")
    bool IsInSilenceMoment() const { return bIsInSilenceMoment; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnTensionStateChanged OnTensionStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnDinosaurVocalization OnDinosaurVocalization;

protected:
    // Core Audio Components
    UPROPERTY()
    class UAudioComponent* MusicAudioComponent;
    
    UPROPERTY()
    class UAudioComponent* AmbienceAudioComponent;
    
    UPROPERTY()
    class UAudioComponent* WeatherAudioComponent;

    // MetaSound Sources
    UPROPERTY(EditDefaultsOnly, Category = "Audio|MetaSounds")
    class UMetaSoundSource* AdaptiveMusicMetaSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio|MetaSounds")
    class UMetaSoundSource* EnvironmentAmbienceMetaSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio|MetaSounds")
    class UMetaSoundSource* TensionSystemMetaSound;

    // Tension System
    UPROPERTY()
    float CurrentTensionLevel = 0.0f;
    
    UPROPERTY()
    ETensionState CurrentTensionState = ETensionState::Calm;
    
    UPROPERTY()
    float TensionDecayRate = 0.1f;

    // Environment State
    UPROPERTY()
    EEnvironmentState CurrentEnvironmentState = EEnvironmentState::DayCalm;
    
    UPROPERTY()
    float CurrentTimeOfDay = 0.5f; // 0 = midnight, 0.5 = noon
    
    UPROPERTY()
    float CurrentWeatherIntensity = 0.0f;

    // Registered Creatures
    UPROPERTY()
    TArray<class ADinosaur*> RegisteredDinosaurs;

    // Silence System
    UPROPERTY()
    bool bIsInSilenceMoment = false;
    
    UPROPERTY()
    FTimerHandle SilenceTimerHandle;

private:
    void UpdateTensionState();
    void UpdateAdaptiveMusic();
    void UpdateEnvironmentalAudio();
    void TickTensionDecay();
    void EndSilenceMoment();
    
    FTimerHandle TensionDecayTimerHandle;
};