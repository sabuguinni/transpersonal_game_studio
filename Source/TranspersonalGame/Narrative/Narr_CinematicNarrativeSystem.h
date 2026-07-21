#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "Narr_CinematicNarrativeSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_CinematicType : uint8
{
    IntroSequence UMETA(DisplayName = "Intro Sequence"),
    ChapterTransition UMETA(DisplayName = "Chapter Transition"),
    MajorEvent UMETA(DisplayName = "Major Event"),
    EmergencyBroadcast UMETA(DisplayName = "Emergency Broadcast"),
    EnvironmentalNarration UMETA(DisplayName = "Environmental Narration"),
    ThreatWarning UMETA(DisplayName = "Threat Warning")
};

USTRUCT(BlueprintType)
struct FNarr_CinematicSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    ENarr_CinematicType CinematicType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    TSoftObjectPtr<USoundBase> VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    TSoftObjectPtr<UTexture2D> BackgroundImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    bool bSkippable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
    TArray<FString> TriggerConditions;

    FNarr_CinematicSequence()
    {
        SequenceID = TEXT("");
        CinematicType = ENarr_CinematicType::IntroSequence;
        NarrativeText = FText::GetEmpty();
        Duration = 10.0f;
        bSkippable = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicStarted, const FString&, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicCompleted, const FString&, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicSkipped, const FString&, SequenceID);

/**
 * Cinematic Narrative System - Manages story-driven cutscenes and narrative sequences
 * Handles intro sequences, chapter transitions, and environmental storytelling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_CinematicNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_CinematicNarrativeSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cinematic sequence management
    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void PlayCinematicSequence(const FString& SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void SkipCurrentCinematic();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void RegisterCinematicSequence(const FNarr_CinematicSequence& Sequence);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    bool IsCinematicPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    FString GetCurrentCinematicID() const;

    // Narrative progression
    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void TriggerIntroSequence();

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void TriggerChapterTransition(const FString& ChapterName);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void TriggerEmergencyBroadcast(const FString& ThreatType);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Narrative")
    void TriggerEnvironmentalNarration(const FString& LocationContext);

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicStarted OnCinematicStarted;

    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicCompleted OnCinematicCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Cinematic Events")
    FOnCinematicSkipped OnCinematicSkipped;

private:
    // Cinematic data
    UPROPERTY()
    TMap<FString, FNarr_CinematicSequence> CinematicSequences;

    UPROPERTY()
    FString CurrentCinematicID;

    UPROPERTY()
    bool bCinematicActive;

    UPROPERTY()
    float CinematicStartTime;

    UPROPERTY()
    UAudioComponent* CinematicAudioComponent;

    // Internal methods
    void InitializeDefaultSequences();
    void OnCinematicTimerComplete();
    void CleanupCurrentCinematic();
    bool CheckTriggerConditions(const TArray<FString>& Conditions);

    // Timer handle for cinematic duration
    FTimerHandle CinematicTimerHandle;
};