#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "MetasoundSource.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudioState : uint8
{
    Exploration,
    Tension,
    Danger,
    Combat,
    Safety,
    Discovery,
    Stealth
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
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Tension = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Safety = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToNearestThreat = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType CurrentEnvironment = EEnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Morning;
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAudioState(const FAudioStateData& NewStateData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAudioState(EAudioState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDinosaurSound(class ADinosaur* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalSound(const FVector& Location, const FString& SoundID, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSFXVolume(float Volume);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateEnvironmentalAudio(EEnvironmentType Environment, ETimeOfDay TimeOfDay);

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerMusicTransition(EAudioState TargetState, float TransitionDuration = 3.0f);

protected:
    // Current audio state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EAudioState CurrentAudioState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentStateData;

    // MetaSound Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> EnvironmentalAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> DinosaurAudioMetaSound;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UAudioComponent> MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UAudioComponent> AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UAudioComponent> SFXAudioComponent;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.8f;

    // Transition parameters
    float StateTransitionTime = 0.0f;
    float MaxTransitionTime = 2.0f;
    EAudioState TargetAudioState;
    bool bIsTransitioning = false;

private:
    void InitializeAudioComponents();
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    void ProcessAudioTransition(float DeltaTime);
    float CalculateTensionLevel() const;
    float CalculateIntensityLevel() const;
};