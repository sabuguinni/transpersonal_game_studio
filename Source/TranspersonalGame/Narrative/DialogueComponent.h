#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "NarrativeTypes.h"
#include "DialogueComponent.generated.h"

class UConsciousnessComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, const FString&, SpeakerName, const FText&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, const FString&, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerChoiceAvailable, const TArray<FNarr_DialogueLine>&, Choices, int32, DefaultChoice, float, TimeLimit);

/**
 * Component that manages dialogue interactions for NPCs
 * Integrates with consciousness system to provide contextual dialogue
 */
UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Start a dialogue conversation with this NPC */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* PlayerActor);

    /** End the current dialogue conversation */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    /** Select a player dialogue choice */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectPlayerChoice(int32 ChoiceIndex);

    /** Check if dialogue is currently active */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    /** Get the current speaker name */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FString GetCurrentSpeaker() const { return CurrentSpeakerName; }

    /** Get available dialogue lines based on player's consciousness level */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FNarr_DialogueLine> GetAvailableDialogue(ENarr_ConsciousnessLevel PlayerConsciousnessLevel) const;

    /** Add a new dialogue line to this character */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueLine(const FNarr_DialogueLine& NewLine);

    /** Remove a dialogue line by index */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RemoveDialogueLine(int32 LineIndex);

    /** Set the character data for this dialogue component */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCharacterData(const FNarr_CharacterData& InCharacterData);

    /** Get the character data */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    const FNarr_CharacterData& GetCharacterData() const { return CharacterData; }

    /** Events */
    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnPlayerChoiceAvailable OnPlayerChoiceAvailable;

protected:
    /** Character data containing dialogue lines and personality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FNarr_CharacterData CharacterData;

    /** Data table containing additional dialogue lines */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    UDataTable* DialogueDataTable;

    /** Maximum interaction distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float MaxInteractionDistance;

    /** Whether dialogue is currently active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsDialogueActive;

    /** Current dialogue line index */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    int32 CurrentDialogueIndex;

    /** Current speaker name */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FString CurrentSpeakerName;

    /** Reference to the player actor currently in dialogue */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentPlayerActor;

    /** Timer for automatic dialogue progression */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    float DialogueTimer;

    /** Current dialogue line being displayed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    FNarr_DialogueLine CurrentDialogueLine;

    /** Available player choices for current dialogue */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_DialogueLine> CurrentPlayerChoices;

private:
    /** Process the next dialogue line */
    void ProcessNextDialogueLine();

    /** Find player choices based on current context */
    void FindPlayerChoices();

    /** Check if a dialogue line meets requirements */
    bool MeetsDialogueRequirements(const FNarr_DialogueLine& DialogueLine, ENarr_ConsciousnessLevel PlayerLevel) const;

    /** Get player's consciousness level */
    ENarr_ConsciousnessLevel GetPlayerConsciousnessLevel(AActor* PlayerActor) const;

    /** Play audio for dialogue line */
    void PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine);

    /** Load dialogue from data table */
    void LoadDialogueFromDataTable();
};