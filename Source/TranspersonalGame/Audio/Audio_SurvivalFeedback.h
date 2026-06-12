#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Audio_SurvivalFeedback.generated.h"

UENUM(BlueprintType)
enum class EAudio_SurvivalState : uint8
{
    Healthy         UMETA(DisplayName = "Healthy"),
    Tired           UMETA(DisplayName = "Low Stamina"),
    Hungry          UMETA(DisplayName = "Hungry"),
    Thirsty         UMETA(DisplayName = "Thirsty"),
    Injured         UMETA(DisplayName = "Injured"),
    Panicked        UMETA(DisplayName = "High Fear"),
    Critical        UMETA(DisplayName = "Critical Condition")
};

UENUM(BlueprintType)
enum class EAudio_ActionType : uint8
{
    Footstep        UMETA(DisplayName = "Footstep"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Eating          UMETA(DisplayName = "Eating"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Combat          UMETA(DisplayName = "Combat"),
    Climbing        UMETA(DisplayName = "Climbing"),
    Swimming        UMETA(DisplayName = "Swimming")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SurvivalSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    EAudio_SurvivalState SurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float HeartbeatRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float BreathingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float Volume;

    FAudio_SurvivalSound()
    {
        SurvivalState = EAudio_SurvivalState::Healthy;
        HeartbeatRate = 1.0f;
        BreathingRate = 1.0f;
        Volume = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ActionSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Audio")
    EAudio_ActionType ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Audio")
    TArray<TSoftObjectPtr<USoundCue>> SoundVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Audio")
    float PitchVariation;

    FAudio_ActionSound()
    {
        ActionType = EAudio_ActionType::Footstep;
        BaseVolume = 0.8f;
        PitchVariation = 0.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SurvivalFeedback : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SurvivalFeedback();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* HeartbeatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BreathingAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* ActionAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TArray<FAudio_SurvivalSound> SurvivalSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TArray<FAudio_ActionSound> ActionSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    EAudio_SurvivalState CurrentSurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    float MasterSurvivalVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    bool bEnableHeartbeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    bool bEnableBreathing;

public:
    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void UpdateSurvivalState(EAudio_SurvivalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayActionSound(EAudio_ActionType ActionType, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void SetSurvivalAudioEnabled(bool bHeartbeat, bool bBreathing);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void UpdateStatsAudio(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayFootstepSound(const FString& SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Survival Audio")
    void PlayPanicSound(float IntensityLevel);

private:
    void InitializeSurvivalSounds();
    void InitializeActionSounds();
    void UpdateHeartbeatAudio();
    void UpdateBreathingAudio();
    FAudio_SurvivalSound* GetSurvivalSoundData(EAudio_SurvivalState State);
    FAudio_ActionSound* GetActionSoundData(EAudio_ActionType ActionType);
    EAudio_SurvivalState CalculateSurvivalState(float Health, float Hunger, float Thirst, 
                                               float Stamina, float Fear);

    float HeartbeatTimer;
    float BreathingTimer;
    bool bIsInitialized;
};