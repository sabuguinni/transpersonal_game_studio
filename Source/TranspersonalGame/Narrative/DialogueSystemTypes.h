// DialogueSystemTypes.h
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Shared types for the dialogue tree system — all 3 quest NPCs use these structs.
// MUST be included before DialogueManager.h

#pragma once

#include "CoreMinimal.h"
#include "Quest/QuestSystemTypes.h"
#include "DialogueSystemTypes.generated.h"

// ─────────────────────────────────────────────────────────────
// ENarr_DialogueNodeType — what kind of node this is in the tree
// ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    NPCSpeech       UMETA(DisplayName = "NPC Speech"),       // NPC delivers a line
    PlayerChoice    UMETA(DisplayName = "Player Choice"),    // Player selects a response
    QuestTrigger    UMETA(DisplayName = "Quest Trigger"),    // Starts/advances a quest
    QuestComplete   UMETA(DisplayName = "Quest Complete"),   // Marks quest complete
    Conditional     UMETA(DisplayName = "Conditional"),      // Branch on game state
    End             UMETA(DisplayName = "End")               // Conversation closes
};

// ─────────────────────────────────────────────────────────────
// ENarr_NPCEmotion — drives animation state on the NPC
// ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_NPCEmotion : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Urgent      UMETA(DisplayName = "Urgent"),
    Fearful     UMETA(DisplayName = "Fearful"),
    Grieving    UMETA(DisplayName = "Grieving"),
    Relieved    UMETA(DisplayName = "Relieved"),
    Angry       UMETA(DisplayName = "Angry"),
    Grateful    UMETA(DisplayName = "Grateful")
};

// ─────────────────────────────────────────────────────────────
// ENarr_QuestDialogueID — links dialogue trees to quest givers
// ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_QuestDialogueID : uint8
{
    None                UMETA(DisplayName = "None"),
    Quest1_Stampede     UMETA(DisplayName = "Quest 1 — Stampede Warning"),
    Quest2_DefendCamp   UMETA(DisplayName = "Quest 2 — Defend the Camp"),
    Quest3_LostChild    UMETA(DisplayName = "Quest 3 — Find the Lost Child"),
    Quest3_ChildFound   UMETA(DisplayName = "Quest 3 — Child Found Reaction"),
    Quest1_Complete     UMETA(DisplayName = "Quest 1 — Completion Debrief"),
    Quest2_Complete     UMETA(DisplayName = "Quest 2 — Completion Debrief"),
    Quest3_Complete     UMETA(DisplayName = "Quest 3 — Completion Debrief")
};

// ─────────────────────────────────────────────────────────────
// FNarr_PlayerOption — one selectable response in a choice node
// ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNarr_PlayerOption
{
    GENERATED_BODY()

    // Text shown on the choice button
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText OptionText;

    // Node ID to jump to when this option is selected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeID = -1;

    // If true, this option is only shown when a condition is met
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bConditional = false;

    // Condition tag — evaluated by DialogueManager at runtime
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionTag = NAME_None;
};

// ─────────────────────────────────────────────────────────────
// FNarr_DialogueNode — one node in a dialogue tree
// ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    // Unique ID within this dialogue tree (0 = root)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueNodeType NodeType = ENarr_DialogueNodeType::NPCSpeech;

    // The spoken/displayed text
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText SpeechText;

    // NPC emotional state while delivering this line
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCEmotion Emotion = ENarr_NPCEmotion::Neutral;

    // Audio asset path (pre-baked ElevenLabs MP3 URL stored as metadata)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    // For NPCSpeech: auto-advance to this node after speech ends (-1 = wait)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 AutoNextNodeID = -1;

    // For PlayerChoice: list of options shown to player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_PlayerOption> PlayerOptions;

    // For QuestTrigger: which quest to start/advance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_QuestDialogueID QuestLink = ENarr_QuestDialogueID::None;

    // Camera focus hint — offset from NPC for cinematic framing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FVector CameraFocusOffset = FVector(200.f, 0.f, 60.f);
};

// ─────────────────────────────────────────────────────────────
// FNarr_DialogueTree — a complete conversation for one NPC
// ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    // Which quest NPC owns this tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_QuestDialogueID DialogueID = ENarr_QuestDialogueID::None;

    // Actor tag to bind this tree to a spawned NPC in the level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NPCActorTag = NAME_None;

    // All nodes in this tree, keyed by NodeID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    // Node to start from (usually 0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 RootNodeID = 0;

    // True once the player has completed this dialogue at least once
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bCompleted = false;
};
