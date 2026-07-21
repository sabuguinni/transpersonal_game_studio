#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueTreeData.generated.h"

// ============================================================
// ENarr_DialogueNodeType — type of node in dialogue tree
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    NPC_Line        UMETA(DisplayName = "NPC Line"),
    PlayerChoice    UMETA(DisplayName = "Player Choice"),
    Condition       UMETA(DisplayName = "Condition Check"),
    Outcome         UMETA(DisplayName = "Outcome / Event")
};

// ============================================================
// ENarr_DialogueCondition — condition types for branching
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    HasItem         UMETA(DisplayName = "Player Has Item"),
    QuestActive     UMETA(DisplayName = "Quest Is Active"),
    QuestComplete   UMETA(DisplayName = "Quest Is Complete"),
    StatThreshold   UMETA(DisplayName = "Stat Above Threshold"),
    FirstMeeting    UMETA(DisplayName = "First Meeting"),
    DangerNearby    UMETA(DisplayName = "Danger Nearby")
};

// ============================================================
// ENarr_DialogueOutcome — what happens after a node resolves
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueOutcome : uint8
{
    None            UMETA(DisplayName = "None"),
    StartQuest      UMETA(DisplayName = "Start Quest"),
    GiveItem        UMETA(DisplayName = "Give Item to Player"),
    TakeItem        UMETA(DisplayName = "Take Item from Player"),
    UpdateReputation UMETA(DisplayName = "Update Reputation"),
    PlayAnimation   UMETA(DisplayName = "Play NPC Animation"),
    EndDialogue     UMETA(DisplayName = "End Dialogue")
};

// ============================================================
// FNarr_DialogueChoice — a single player response option
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition RequiredCondition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionParam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bHideIfConditionFails = true;
};

// ============================================================
// FNarr_DialogueNode — a single node in the dialogue tree
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueNodeType NodeType = ENarr_DialogueNodeType::NPC_Line;

    // NPC line text (used when NodeType == NPC_Line)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText SpeakerText;

    // Speaker name tag (e.g., "HunterLeader", "CampElder")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SpeakerID;

    // Optional audio URL (ElevenLabs TTS output)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    // Player choices (used when NodeType == PlayerChoice)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    // Condition to evaluate (used when NodeType == Condition)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionParam;

    // Branch targets: index 0 = condition true, index 1 = condition false
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 TrueNodeIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 FalseNodeIndex = -1;

    // Outcome to trigger when this node resolves
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueOutcome Outcome = ENarr_DialogueOutcome::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName OutcomeParam;

    // Auto-advance to next node (no player input needed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bAutoAdvance = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float AutoAdvanceDelay = 2.0f;
};

// ============================================================
// UNarr_DialogueTree — Data Asset holding a full dialogue tree
// ============================================================
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueTree : public UDataAsset
{
    GENERATED_BODY()

public:

    // Unique ID for this dialogue tree (e.g., "DLG_HunterLeader_MainQuest")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName DialogueID;

    // NPC this dialogue belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName OwnerNPC_ID;

    // All nodes in this tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    // Index of the entry node (usually 0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 EntryNodeIndex = 0;

    // Whether this dialogue can be replayed after completion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bReplayable = false;

    // Quest ID to start when this dialogue completes (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName QuestToStart;

    // Get node by index (returns nullptr if out of range)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    const FNarr_DialogueNode* GetNode(int32 Index) const;

    // Get entry node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    const FNarr_DialogueNode* GetEntryNode() const;

    // Validate tree integrity (all node indices valid)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Dialogue")
    bool ValidateTree() const;
};
