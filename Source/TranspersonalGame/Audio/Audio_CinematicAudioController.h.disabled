#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_CinematicAudioController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_CinematicSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    TSoftObjectPtr<USoundWave> VoiceNarration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    TSoftObjectPtr<USoundWave> BackgroundMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    TSoftObjectPtr<USoundWave> EnvironmentalAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    float SequenceDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    float VoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    float AmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    bool bCanBeSkipped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    bool bFadeInMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    bool bFadeOutMusic;

    FAudio_CinematicSequence()
    {
        SequenceName = TEXT("DefaultSequence");
        SequenceDuration = 30.0f;
        VoiceVolume = 1.0f;
        MusicVolume = 0.6f;
        AmbienceVolume = 0.4f;
        bCanBeSkipped = true;
        bFadeInMusic = true;
        bFadeOutMusic = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_CinematicAudioController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_CinematicAudioController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceAudioComponent;

    // Cinematic Sequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Audio")
    TArray<FAudio_CinematicSequence> CinematicSequences;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    int32 CurrentSequenceIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsPlayingSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentSequenceTime;

    // Timer Handles
    FTimerHandle SequenceTimerHandle;
    FTimerHandle FadeTimerHandle;

public:
    // Sequence Control
    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void PlaySequence(int32 SequenceIndex);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void PlaySequenceByName(const FString& SequenceName);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void StopCurrentSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void SkipCurrentSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void PlayNextSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void PlayPreviousSequence();

    // Audio Control
    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void SetVoiceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void SetAmbienceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void FadeInMusic(float FadeDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void FadeOutMusic(float FadeDuration = 2.0f);

    // Sequence Management
    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void AddSequence(const FAudio_CinematicSequence& NewSequence);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void RemoveSequence(int32 SequenceIndex);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Audio")
    void ClearAllSequences();

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Cinematic Audio")
    bool IsPlayingSequence() const { return bIsPlayingSequence; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Audio")
    int32 GetCurrentSequenceIndex() const { return CurrentSequenceIndex; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Audio")
    float GetCurrentSequenceTime() const { return CurrentSequenceTime; }

    UFUNCTION(BlueprintPure, Category = "Cinematic Audio")
    int32 GetSequenceCount() const { return CinematicSequences.Num(); }

    UFUNCTION(BlueprintPure, Category = "Cinematic Audio")
    FAudio_CinematicSequence GetSequence(int32 SequenceIndex) const;

private:
    // Internal Methods
    void StartSequence(int32 SequenceIndex);
    void EndSequence();
    void UpdateSequenceTimer();
    void HandleFadeTimer();
    void LoadSequenceAudio(const FAudio_CinematicSequence& Sequence);

    // Fade State
    bool bIsFading;
    float FadeStartVolume;
    float FadeTargetVolume;
    float FadeDuration;
    float FadeElapsedTime;
    UAudioComponent* FadeTargetComponent;
};