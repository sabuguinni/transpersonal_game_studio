#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_AdaptiveMusicSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Calm,
    Tension,
    Combat,
    Exploration,
    Storytelling,
    Night,
    Day
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest,
    Plains,
    Mountains,
    Swamp,
    Desert,
    River
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TSoftObjectPtr<USoundWave> SoundWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState MusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Volume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeOutTime = 2.0f;

    FAudio_MusicTrack()
    {
        TrackName = TEXT("DefaultTrack");
        MusicState = EAudio_MusicState::Calm;
        BiomeType = EAudio_BiomeType::Forest;
        Volume = 0.7f;
        bIsLooping = true;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_StorytellingAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float Duration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bIsNarration = true;

    FAudio_StorytellingAudioData()
    {
        AudioURL = TEXT("");
        CharacterName = TEXT("Narrator");
        Duration = 10.0f;
        bIsNarration = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Music State Management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetBiomeType(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void PlayStorytellingAudio(const FAudio_StorytellingAudioData& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopStorytellingAudio();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void FadeToTrack(const FAudio_MusicTrack& NewTrack, float FadeTime = 2.0f);

    // Tension System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void IncreaseTension(float Amount = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void DecreaseTension(float Amount = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void ResetTension();

    // Day/Night Cycle Integration
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnDayNightTransition(bool bIsNight);

    // Dinosaur Proximity System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnDinosaurProximity(float Distance, bool bIsPredator);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void OnDinosaurLeft();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Adaptive Music")
    EAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintPure, Category = "Adaptive Music")
    float GetTensionLevel() const { return TensionLevel; }

    UFUNCTION(BlueprintPure, Category = "Adaptive Music")
    bool IsStorytellingActive() const { return bIsStorytellingActive; }

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Tracks")
    TArray<FAudio_MusicTrack> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    UPROPERTY(meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> StorytellingAudioComponent;

    // State Variables
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAudio_BiomeType CurrentBiomeType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float TensionLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsStorytellingActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsNightTime;

    // Audio Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float StorytellingVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float TensionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DinosaurProximityThreshold;

    // Timers and Transitions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transitions")
    float CurrentFadeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transitions")
    bool bIsFading;

    FTimerHandle StorytellingTimerHandle;
    FTimerHandle TensionDecayTimerHandle;

private:
    // Internal Methods
    void UpdateMusicBasedOnState();
    FAudio_MusicTrack* FindBestTrackForCurrentState();
    void StartFadeTransition(const FAudio_MusicTrack& NewTrack, float FadeTime);
    void OnStorytellingFinished();
    void DecayTensionOverTime();
    void InitializeAudioComponents();
    void LoadDefaultMusicTracks();
};