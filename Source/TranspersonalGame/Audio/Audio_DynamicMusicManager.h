#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_DynamicMusicManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Exploration UMETA(DisplayName = "Exploration"), 
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Victory     UMETA(DisplayName = "Victory"),
    Death       UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EAudio_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstContact    UMETA(DisplayName = "First Contact"),
    Survival        UMETA(DisplayName = "Survival"),
    Discovery       UMETA(DisplayName = "Discovery"),
    TribeDiscovery  UMETA(DisplayName = "Tribe Discovery"),
    Conflict        UMETA(DisplayName = "Conflict"),
    Resolution      UMETA(DisplayName = "Resolution")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState MusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_StoryPhase StoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bLooping;

    FAudio_MusicTrack()
    {
        MusicState = EAudio_MusicState::Calm;
        StoryPhase = EAudio_StoryPhase::Awakening;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bLooping = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_DynamicMusicManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DynamicMusicManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* PrimaryMusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SecondaryMusicComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Tracks")
    TArray<FAudio_MusicTrack> MusicTracks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    EAudio_StoryPhase CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MasterMusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float CrossfadeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsTransitioning;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetStoryPhase(EAudio_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void PlayMusicForStateAndPhase(EAudio_MusicState State, EAudio_StoryPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void StopAllMusic();

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Music State")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintPure, Category = "Music State")
    EAudio_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

protected:
    UFUNCTION()
    void OnMusicFinished();

    void CrossfadeToTrack(const FAudio_MusicTrack& NewTrack);
    FAudio_MusicTrack* FindTrackForStateAndPhase(EAudio_MusicState State, EAudio_StoryPhase Phase);
    void InitializeDefaultTracks();
};