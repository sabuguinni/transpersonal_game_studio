#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeSystem.generated.h"

// Forward declarations
class UDialogueComponent;
class ANarrativeActor;

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForInput,
    Playing,
    Completed
};

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player learns basic survival
    Discovery,      // First encounters with major threats
    Adaptation,     // Learning to thrive in the world
    Mastery,        // Becoming apex survivor
    Legacy          // Establishing permanent settlement
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT(""));
        Duration = 3.0f;
        AudioPath = TEXT("");
        bRequiresPlayerResponse = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = FText::FromString(TEXT(""));
        EventDescription = FText::FromString(TEXT(""));
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryEventCompleted(const FString& EventID) const;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const TArray<FNarr_DialogueLine>& DialogueLines);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_DialogueState GetDialogueState() const { return CurrentDialogueState; }

    // Environmental storytelling
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNarrativeActor(ANarrativeActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterNarrativeActor(ANarrativeActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarrative(const FVector& Location, float Radius);

    // Survival narrative integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerSurvivalEvent(const FString& EventType, float Severity);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurEncounter(const FString& DinosaurType, bool bHostile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnCraftingAchievement(const FString& ItemCrafted);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentDialogueState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueLine> CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<TWeakObjectPtr<ANarrativeActor>> RegisteredNarrativeActors;

    // Timer for dialogue progression
    FTimerHandle DialogueTimerHandle;

private:
    void InitializeStoryEvents();
    void ProcessDialogueTimer();
    void BroadcastStoryPhaseChange(ENarr_StoryPhase NewPhase);
};