#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetFlags;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletionFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        bIsCompleted = false;
    }
};

UENUM(BlueprintType)
enum class ENarr_NarrativeState : uint8
{
    Intro           UMETA(DisplayName = "Introduction"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    Survival        UMETA(DisplayName = "Survival Phase"),
    Leadership      UMETA(DisplayName = "Leadership Phase"),
    Mastery         UMETA(DisplayName = "Mastery Phase")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeEvent, const FString&, EventID, const FString&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FNarr_DialogueEntry&, DialogueEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Narrative Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, const FString& EventData = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeFlag(const FString& FlagName, bool bValue = true);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetNarrativeFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    // Narrative State Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeState(ENarr_NarrativeState NewState);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_NarrativeState GetCurrentNarrativeState() const { return CurrentNarrativeState; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueEnded OnDialogueEnded;

protected:
    // Data Tables
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    TSoftObjectPtr<UDataTable> DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    TSoftObjectPtr<UDataTable> StoryBeatsDataTable;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, bool> NarrativeFlags;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TMap<FString, FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_NarrativeState CurrentNarrativeState;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FNarr_DialogueEntry CurrentDialogue;

    // Internal Functions
    void LoadNarrativeData();
    void InitializeStoryBeats();
    bool CheckConditions(const TArray<FString>& Conditions) const;
    void ProcessCompletionFlags(const TArray<FString>& Flags);

private:
    FTimerHandle DialogueTimerHandle;
    void OnDialogueTimeout();
};