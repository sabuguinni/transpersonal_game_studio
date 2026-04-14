#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NarrativeTypes.h"
#include "DialogueComponent.generated.h"

class UAudioComponent;
class UNarrativeManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, DialogueID, const FString&, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueLineChanged, const FString&, DialogueText, const FString&, SpeakerName, ENarr_EmotionalState, EmotionalState);

/**
 * Component that handles dialogue interactions for NPCs and story triggers
 */
UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    /** Start a dialogue sequence */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(const FString& DialogueID, AActor* InteractingActor = nullptr);

    /** End the current dialogue */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    /** Advance to the next dialogue line */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool AdvanceDialogue();

    /** Select a dialogue choice (for branching conversations) */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectDialogueChoice(int32 ChoiceIndex);

    /** Get available dialogue choices */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetDialogueChoices() const;

    /** Check if dialogue is currently active */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    /** Get current dialogue line */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const { return CurrentDialogueLine; }

    /** Set the dialogue data table */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetDialogueDataTable(UDataTable* NewDialogueTable);

    /** Play audio for current dialogue line */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio();

    /** Stop current dialogue audio */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopDialogueAudio();

    /** Events */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueLineChanged OnDialogueLineChanged;

protected:
    /** Data table containing dialogue lines */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    UDataTable* DialogueDataTable;

    /** Character archetype for this dialogue component */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterArchetype CharacterArchetype;

    /** Default speaker name if not specified in dialogue data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString DefaultSpeakerName;

    /** Audio component for playing dialogue sounds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* AudioComponent;

    /** Current dialogue state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsDialogueActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FNarr_DialogueLine CurrentDialogueLine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FString CurrentDialogueID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TArray<FString> CurrentChoices;

    /** Reference to the narrative manager */
    UPROPERTY()
    UNarrativeManager* NarrativeManager;

private:
    /** Load dialogue line from data table */
    bool LoadDialogueLine(const FString& DialogueID);

    /** Process dialogue line for consciousness level requirements */
    bool CanAccessDialogueLine(const FNarr_DialogueLine& DialogueLine) const;

    /** Get player consciousness level */
    int32 GetPlayerConsciousnessLevel() const;

    /** Update dialogue choices based on current line */
    void UpdateDialogueChoices();
};