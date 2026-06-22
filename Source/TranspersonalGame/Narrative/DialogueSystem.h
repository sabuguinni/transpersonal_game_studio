// DialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260622_005
// Dialogue tree system for NPC interactions in the prehistoric survival world.
// Integrates with QuestManager (Agent #14) for quest-gated dialogue options.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueSystem.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Player      UMETA(DisplayName = "Player"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Stranger    UMETA(DisplayName = "Stranger"),
    Narrator    UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    QuestActive     UMETA(DisplayName = "QuestActive"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    QuestFailed     UMETA(DisplayName = "QuestFailed"),
    HasItem         UMETA(DisplayName = "HasItem"),
    FirstMeeting    UMETA(DisplayName = "FirstMeeting"),
    DayTime         UMETA(DisplayName = "DayTime"),
    NightTime       UMETA(DisplayName = "NightTime")
};

UENUM(BlueprintType)
enum class ENarr_DialogueEffect : uint8
{
    None            UMETA(DisplayName = "None"),
    StartQuest      UMETA(DisplayName = "StartQuest"),
    CompleteQuest   UMETA(DisplayName = "CompleteQuest"),
    GiveItem        UMETA(DisplayName = "GiveItem"),
    OpenTrade       UMETA(DisplayName = "OpenTrade"),
    EndConversation UMETA(DisplayName = "EndConversation"),
    TriggerEvent    UMETA(DisplayName = "TriggerEvent")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::Narrator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    // Optional voice asset path (ElevenLabs generated audio)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString VoiceAssetPath;

    // How long to display this line (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;

    // Emotion/animation hint for the NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName AnimationHint;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    // Node ID this choice leads to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionParam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueEffect Effect = ENarr_DialogueEffect::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName EffectParam;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    // If no choices, auto-advance to this node (empty = end conversation)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName AutoNextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition EntryCondition = ENarr_DialogueCondition::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName EntryConditionParam;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID;

    // NPC this tree belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker OwnerNPC = ENarr_DialogueSpeaker::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName StartNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    // Has the player spoken to this NPC before?
    UPROPERTY(BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bFirstMeetingDone = false;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDialogueSystem : public UObject
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    // Initialize all NPC dialogue trees
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void InitializeDialogueTrees();

    // Start a conversation with an NPC
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(ENarr_DialogueSpeaker NPC);

    // Advance to next line in current node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    // Player selects a choice (returns false if invalid index)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool SelectChoice(int32 ChoiceIndex);

    // End the current conversation
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    // Get current line being displayed
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    // Get available choices at current node
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    // Is a conversation currently active?
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    // Get all registered trees
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    int32 GetDialogueTreeCount() const;

    // Mark NPC first meeting as done
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SetFirstMeetingDone(ENarr_DialogueSpeaker NPC);

    // Check if player has met this NPC before
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasMetNPC(ENarr_DialogueSpeaker NPC) const;

private:
    // All dialogue trees keyed by NPC speaker
    UPROPERTY()
    TMap<uint8, FNarr_DialogueTree> DialogueTrees;

    // Currently active tree
    UPROPERTY()
    FNarr_DialogueTree ActiveTree;

    // Currently active node ID
    UPROPERTY()
    FName ActiveNodeID;

    // Current line index within the active node
    UPROPERTY()
    int32 CurrentLineIndex = 0;

    // Is dialogue running?
    UPROPERTY()
    bool bDialogueActive = false;

    // Internal helpers
    FNarr_DialogueNode* FindNode(FNarr_DialogueTree& Tree, FName NodeID);
    bool EvaluateCondition(ENarr_DialogueCondition Condition, FName Param) const;
    void ApplyEffect(ENarr_DialogueEffect Effect, FName Param);

    // Tree builders
    void BuildElderTree();
    void BuildScoutTree();
    void BuildCrafterTree();
};
