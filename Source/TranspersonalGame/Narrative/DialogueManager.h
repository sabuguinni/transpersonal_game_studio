// DialogueManager.h — Narrative & Dialogue Agent #15
// PROD_CYCLE_AUTO_20260620_006
// UGameInstanceSubsystem managing all NPC dialogue trees.
// Integrates with QuestManager via OnQuestShouldStart delegate.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DialogueManager.generated.h"

// ─────────────────────────────────────────────────────────────
//  Structs — all prefixed Narr_ per UE5 compilation rules
// ─────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeIndex = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName RequiredQuestState = NAME_None;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText SpeakerName;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText DialogueText;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsPlayerChoice = false;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bEndsDialogue = false;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bStartsQuest = false;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioCueID;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName DialogueID = NAME_None;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText NPCName;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName QuestToStartOnComplete = NAME_None;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL_Offer;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL_InProgress;

    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL_Complete;
};

// ─────────────────────────────────────────────────────────────
//  Delegates
// ─────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueStarted, FName, DialogueID, FText, NPCName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueNodeAdvanced, FName, DialogueID, FNarr_DialogueNode, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestShouldStart, FName, QuestID);

// ─────────────────────────────────────────────────────────────
//  UDialogueManager — GameInstance Subsystem
// ─────────────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ── Start / Control ──────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(FName DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FName GetActiveDialogueID() const;

    // ── Query ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueTree GetDialogueTree(FName DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FName> GetAllDialogueIDs() const;

    // ── Delegates ────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnDialogueNodeAdvanced OnDialogueNodeAdvanced;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnQuestShouldStart OnQuestShouldStart;

private:

    void BuildDialogueDatabase();
    void AdvanceToNode(int32 NodeIndex);

    UPROPERTY()
    TMap<FName, FNarr_DialogueTree> DialogueDatabase;

    UPROPERTY()
    FName ActiveDialogueID;

    UPROPERTY()
    int32 ActiveNodeIndex = 0;

    UPROPERTY()
    bool bDialogueActive = false;
};
