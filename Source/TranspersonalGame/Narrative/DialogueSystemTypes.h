#pragma once

#include "CoreMinimal.h"
#include "DialogueSystemTypes.generated.h"

// ============================================================
// DIALOGUE SYSTEM TYPES — Agent #15 Narrative & Dialogue
// All types prefixed with Narr_ to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestOffer      UMETA(DisplayName = "Quest Offer"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    QuestFailed     UMETA(DisplayName = "Quest Failed"),
    Lore            UMETA(DisplayName = "Lore"),
    Farewell        UMETA(DisplayName = "Farewell")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Watcher         UMETA(DisplayName = "Watcher"),
    Villager        UMETA(DisplayName = "Villager"),
    Survivor        UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    Always              UMETA(DisplayName = "Always"),
    QuestNotStarted     UMETA(DisplayName = "Quest Not Started"),
    QuestActive         UMETA(DisplayName = "Quest Active"),
    QuestCompleted      UMETA(DisplayName = "Quest Completed"),
    QuestFailed         UMETA(DisplayName = "Quest Failed"),
    PlayerHasItem       UMETA(DisplayName = "Player Has Item"),
    PlayerHealthLow     UMETA(DisplayName = "Player Health Low"),
    TimeOfDay_Day       UMETA(DisplayName = "Time of Day: Day"),
    TimeOfDay_Night     UMETA(DisplayName = "Time of Day: Night"),
    DinosaurNearby      UMETA(DisplayName = "Dinosaur Nearby")
};

// A single line of dialogue with condition and audio reference
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    // Unique ID for this line (used for audio cue lookup)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LineID;

    // The spoken text
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Condition that must be true for this line to play
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::Always;

    // Optional quest ID this line is associated with
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName AssociatedQuestID;

    // Audio asset path (e.g. /Game/Audio/Dialogue/TribalElder_Greeting)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    // Subtitle display duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float SubtitleDuration = 4.0f;

    // Should this line trigger a quest offer?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bTriggersQuestOffer = false;

    // Quest ID to offer if bTriggersQuestOffer is true
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName QuestToOffer;
};

// A node in the dialogue tree — one NPC utterance + player response options
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    // Unique node ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NodeID;

    // The NPC line spoken at this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueLine NPCLine;

    // Player response options (empty = end of conversation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> PlayerResponses;

    // Which node each player response leads to (parallel array to PlayerResponses)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FName> ResponseNextNodes;

    // Is this a terminal node (ends the conversation)?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsTerminal = false;

    // State this node transitions the dialogue to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState TransitionState = ENarr_DialogueState::Idle;
};

// Complete dialogue tree for one NPC
USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    // NPC identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NPCID;

    // NPC display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    // NPC role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::Villager;

    // Entry node ID (first node to play when player enters dialogue range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName EntryNodeID;

    // All nodes in this dialogue tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    // Dialogue radius — player must be within this distance to trigger
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DialogueRadius = 250.0f;
};

// Event broadcast when a dialogue line is spoken
USTRUCT(BlueprintType)
struct FNarr_DialogueEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName NPCID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName NodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString LineText;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    ENarr_DialogueState NewState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float Timestamp = 0.0f;
};
