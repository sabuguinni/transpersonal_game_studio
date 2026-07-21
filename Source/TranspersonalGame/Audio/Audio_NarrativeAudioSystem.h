#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_NarrativeAudioSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_NarrativeType : uint8
{
    TribalLegend    UMETA(DisplayName = "Tribal Legend"),
    HuntTale        UMETA(DisplayName = "Hunt Tale"),
    WarningStory    UMETA(DisplayName = "Warning Story"),
    TerritoreLore   UMETA(DisplayName = "Territory Lore"),
    SurvivalWisdom  UMETA(DisplayName = "Survival Wisdom"),
    BeastKnowledge  UMETA(DisplayName = "Beast Knowledge")
};

UENUM(BlueprintType)
enum class EAudio_NarratorType : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    ScoutWarrior    UMETA(DisplayName = "Scout Warrior"),
    HunterVeteran   UMETA(DisplayName = "Hunter Veteran"),
    TrackerGuide    UMETA(DisplayName = "Tracker Guide"),
    CrafterWise     UMETA(DisplayName = "Crafter Wise"),
    TerritoryGuard  UMETA(DisplayName = "Territory Guard")
};

USTRUCT(BlueprintType)
struct FAudio_NarrativeClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_NarrativeType NarrativeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_NarratorType NarratorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<USoundWave> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bIsLooping;

    FAudio_NarrativeClip()
    {
        ClipName = TEXT("");
        NarrativeType = EAudio_NarrativeType::TribalLegend;
        NarratorType = EAudio_NarratorType::TribalElder;
        AudioURL = TEXT("");
        Duration = 0.0f;
        Volume = 1.0f;
        bIsLooping = false;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_NarrativeAudioSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_NarrativeAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Narrative Audio Library
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TArray<FAudio_NarrativeClip> NarrativeClips;

    // Current Playback State
    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    bool bIsPlayingNarrative;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    FAudio_NarrativeClip CurrentClip;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative Audio")
    float CurrentPlaybackTime;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbienceAudioComponent;

    // Narrative Playback Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeByType(EAudio_NarrativeType NarrativeType, EAudio_NarratorType NarratorType);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopCurrentNarrative();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PauseCurrentNarrative();

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void ResumeCurrentNarrative();

    // Audio Mixing Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetNarrativeVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void SetAmbienceVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void FadeInNarrative(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void FadeOutNarrative(float FadeTime = 2.0f);

    // Library Management
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void AddNarrativeClip(const FAudio_NarrativeClip& NewClip);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void RemoveNarrativeClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    FAudio_NarrativeClip GetNarrativeClip(const FString& ClipName);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    TArray<FAudio_NarrativeClip> GetClipsByType(EAudio_NarrativeType NarrativeType);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    TArray<FAudio_NarrativeClip> GetClipsByNarrator(EAudio_NarratorType NarratorType);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeStarted, const FAudio_NarrativeClip&, StartedClip);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeFinished, const FAudio_NarrativeClip&, FinishedClip);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativePaused, const FAudio_NarrativeClip&, PausedClip);

    UPROPERTY(BlueprintAssignable, Category = "Narrative Audio Events")
    FOnNarrativeStarted OnNarrativeStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Audio Events")
    FOnNarrativeFinished OnNarrativeFinished;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Audio Events")
    FOnNarrativePaused OnNarrativePaused;

private:
    // Internal state
    FTimerHandle FadeTimerHandle;
    float TargetVolume;
    float FadeStartVolume;
    float FadeTimeRemaining;
    bool bIsFading;

    // Internal functions
    void UpdateFade();
    void OnNarrativePlaybackFinished();
    void InitializeDefaultClips();
};