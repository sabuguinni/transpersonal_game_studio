// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once
// DISABLED: // DISABLED: #include "TranspersonalGameSharedTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeTypes.h"
#include "GameplayTagContainer.h"
#include "NarrativeManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeEvent, const FNarrativeEvent&, Event, float, EmotionalImpact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueTriggered, const FDialogueLine&, Line, AActor*, Speaker, AActor*, Listener);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryBeatCompleted, const FStoryBeat&, Beat);

/**
 * Central narrative system that manages story progression, emotional arcs, and dialogue
 * Based on Robert McKee's pressure-driven storytelling and Kojima's gameplay-narrative integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Narrative Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, AActor* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FString& LineID, AActor* Speaker, AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    EEmotionalTone GetDominantEmotionalTone() const { return DominantTone; }

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FStoryBeat> GetAvailableStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateEmotionalState(EEmotionalTone NewTone, float Intensity);

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FDialogueLine GetDialogueForContext(const FString& SpeakerID, EDialogueContext Context) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTriggerDialogue(const FString& LineID, AActor* Speaker, AActor* Listener) const;

    // Memory System - tracks player's narrative choices
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RecordPlayerChoice(const FString& ChoiceID, const FString& ChoiceValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetPlayerChoice(const FString& ChoiceID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnStoryBeatCompleted OnStoryBeatCompleted;

protected:
    // Data Tables
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TObjectPtr<UDataTable> NarrativeEventsTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TObjectPtr<UDataTable> DialogueTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TObjectPtr<UDataTable> StoryBeatsTable;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EEmotionalTone DominantTone;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TSet<FString> CompletedStoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TSet<FString> TriggeredEvents;

    // Player Memory System
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, FString> PlayerChoices;

    // Emotional Arc Tracking
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<float> EmotionalHistory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    int32 MaxEmotionalHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float EmotionalDecayRate;

private:
    void InitializeDataTables();
    void UpdateEmotionalHistory(float NewValue);
    bool EvaluateNarrativeConditions(const FGameplayTagContainer& RequiredTags) const;
    void ProcessEmotionalImpact(float Impact, EEmotionalTone Tone);
};