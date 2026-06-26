#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Agent #15 — Narrative & Dialogue Agent
// DialogueSystem.h — NPC dialogue trees for quest givers
// Cycle: PROD_CYCLE_AUTO_20260625_012
// ============================================================

// Enum: which NPC is speaking
UENUM(BlueprintType)
enum class ENarr_NPCIdentity : uint8
{
    None            UMETA(DisplayName = "None"),
    Hunter_Kael     UMETA(DisplayName = "Hunter Kael"),
    Elder_Mara      UMETA(DisplayName = "Elder Mara"),
    Lookout_Dara    UMETA(DisplayName = "Lookout Dara"),
    Stranger        UMETA(DisplayName = "Unknown Stranger")
};

// Enum: dialogue response type (player choice)
UENUM(BlueprintType)
enum class ENarr_DialogueResponse : uint8
{
    Accept          UMETA(DisplayName = "Accept Quest"),
    Decline         UMETA(DisplayName = "Decline"),
    AskMore         UMETA(DisplayName = "Ask for More Info"),
    Barter          UMETA(DisplayName = "Offer Trade"),
    Farewell        UMETA(DisplayName = "Say Goodbye")
};

// Enum: dialogue state in the conversation tree
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestOffer      UMETA(DisplayName = "Quest Offer"),
    QuestDetails    UMETA(DisplayName = "Quest Details"),
    QuestAccepted   UMETA(DisplayName = "Quest Accepted"),
    QuestDeclined   UMETA(DisplayName = "Quest Declined"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    Idle            UMETA(DisplayName = "Idle / No Quest")
};

// A single line of dialogue
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCIdentity Speaker = ENarr_NPCIdentity::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TriggerState = ENarr_DialogueState::Greeting;

    // Optional audio asset path (set by Audio Agent #16)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // How long to display this line (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;
};

// A player response option shown in UI
USTRUCT(BlueprintType)
struct FNarr_PlayerChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueResponse ResponseType = ENarr_DialogueResponse::Accept;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState NextState = ENarr_DialogueState::QuestAccepted;
};

// Full dialogue tree node
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State = ENarr_DialogueState::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_PlayerChoice> PlayerChoices;
};

// ============================================================
// ADialogueSystem — Actor placed near NPCs to drive conversation
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueSystem : public AActor
{
    GENERATED_BODY()

public:
    ADialogueSystem();

    // Which NPC owns this dialogue tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCIdentity NPCIdentity = ENarr_NPCIdentity::None;

    // Current state in the conversation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState CurrentState = ENarr_DialogueState::Greeting;

    // All dialogue nodes for this NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> DialogueTree;

    // Interaction radius (player must be within this distance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius = 300.0f;

    // Is dialogue currently active?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive = false;

    // Start conversation with this NPC
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    // Advance to next state based on player choice
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue(ENarr_DialogueResponse PlayerResponse);

    // End conversation
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get lines for current state
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetCurrentLines() const;

    // Get player choices for current state
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_PlayerChoice> GetCurrentChoices() const;

    // Build default dialogue tree for Hunter Kael (mammoth quest)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void BuildHunterKaelDialogue();

    // Build default dialogue tree for Elder Mara (raptor quest)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void BuildElderMaraDialogue();

    // Build default dialogue tree for Lookout Dara (post-quest debrief)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative")
    void BuildLookoutDaraDialogue();

    // BlueprintImplementableEvent — called when dialogue starts (for UI)
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueStarted(ENarr_NPCIdentity NPC);

    // BlueprintImplementableEvent — called when state changes
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueStateChanged(ENarr_DialogueState NewState);

    // BlueprintImplementableEvent — called when dialogue ends
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative")
    void OnDialogueEnded(ENarr_NPCIdentity NPC);

protected:
    virtual void BeginPlay() override;

private:
    // Find the node for a given state
    const FNarr_DialogueNode* FindNode(ENarr_DialogueState State) const;
};
