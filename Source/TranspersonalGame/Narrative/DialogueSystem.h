#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystem
// Prehistoric survival dialogue — no spiritual content
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Greeting    UMETA(DisplayName = "Greeting"),
    Warning     UMETA(DisplayName = "Warning"),
    QuestGive   UMETA(DisplayName = "QuestGive"),
    QuestDone   UMETA(DisplayName = "QuestDone"),
    Farewell    UMETA(DisplayName = "Farewell")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "TribalElder"),
    ScoutHunter     UMETA(DisplayName = "ScoutHunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    TribalLeader    UMETA(DisplayName = "TribalLeader"),
    Survivor        UMETA(DisplayName = "Survivor")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerID(TEXT("Unknown"))
        , LineText(TEXT(""))
        , TriggerState(ENarr_DialogueState::Idle)
        , DisplayDuration(3.0f)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , NPCRole(ENarr_NPCRole::Survivor)
    {}
};

/**
 * UNarr_DialogueComponent
 * Attach to any NPC actor to give it dialogue capability.
 * Manages state machine for prehistoric survival conversations.
 */
UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent), DisplayName = "Dialogue Component")
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

    // Current dialogue state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState;

    // NPC role determines available dialogue trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    // Dialogue trees loaded for this NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // Interaction radius — player must be within this range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float InteractionRadius;

    // Whether NPC is currently in dialogue with player
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bIsInDialogue;

    // Index of current line being displayed
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    int32 CurrentLineIndex;

    // Begin dialogue interaction
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BeginDialogue(ENarr_DialogueState InitialState);

    // Advance to next dialogue line
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue();

    // End dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get current line text
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative")
    FString GetCurrentLineText() const;

    // Get current speaker ID
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative")
    FString GetCurrentSpeakerID() const;

    // Transition to a new state
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetDialogueState(ENarr_DialogueState NewState);

    // Load default lines for this NPC role
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadDefaultDialogueForRole();

protected:
    virtual void BeginPlay() override;

private:
    // Active tree being played
    FNarr_DialogueTree* ActiveTree;

    // Find tree matching current state
    FNarr_DialogueTree* FindTreeForState(ENarr_DialogueState State);

    // Build default dialogue lines per role
    void BuildElderDialogue();
    void BuildScoutDialogue();
    void BuildGathererDialogue();
};
