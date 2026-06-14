#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"
#include "Audio_CampfireSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_CampfireState : uint8
{
    Unlit UMETA(DisplayName = "Unlit"),
    Crackling UMETA(DisplayName = "Crackling"),
    Storytelling UMETA(DisplayName = "Storytelling"),
    Dying UMETA(DisplayName = "Dying")
};

USTRUCT(BlueprintType)
struct FAudio_CampfireAudioSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float CrackleVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float StorytellingVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float StorytellingRange = 500.0f;

    FAudio_CampfireAudioSettings()
    {
        CrackleVolume = 0.7f;
        StorytellingVolume = 0.5f;
        FadeInDuration = 2.0f;
        FadeOutDuration = 3.0f;
        StorytellingRange = 500.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_CampfireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_CampfireSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void SetCampfireState(EAudio_CampfireState NewState);

    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    EAudio_CampfireState GetCampfireState() const { return CurrentState; }

    // Storytelling Audio
    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void PlayStoryAudio(const FString& AudioURL, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void StopStoryAudio();

    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    bool IsStoryPlaying() const { return bIsStoryPlaying; }

    // Volume Control
    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void SetCrackleVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void SetStoryVolume(float Volume);

    // Proximity Detection
    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    void CheckPlayerProximity();

    UFUNCTION(BlueprintCallable, Category = "Campfire Audio")
    bool IsPlayerInRange() const { return bPlayerInRange; }

protected:
    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CrackleAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* StoryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* CampfireCrackleCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* CampfireAmbientCue;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_CampfireAudioSettings AudioSettings;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAudio_CampfireState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsStoryPlaying;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bPlayerInRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentStoryDuration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float StoryPlaybackTime;

    // Internal Methods
    void InitializeAudioComponents();
    void UpdateAudioBasedOnState();
    void HandleStateTransition(EAudio_CampfireState OldState, EAudio_CampfireState NewState);
    void FadeCrackleAudio(float TargetVolume, float Duration);
    void UpdateStoryPlayback(float DeltaTime);

private:
    // Fade timers
    float FadeTimer;
    float FadeTargetVolume;
    float FadeStartVolume;
    float FadeDuration;
    bool bIsFading;
};