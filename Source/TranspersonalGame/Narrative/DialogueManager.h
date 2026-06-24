// DialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260624_009
// UWorldSubsystem managing dialogue trees, NPC voice lines, and quest-linked narrative branches.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive     UMETA(DisplayName = "Inactive"),
    Active       UMETA(DisplayName = "Active"),
    WaitingInput UMETA(DisplayName = "WaitingInput"),
    Completed    UMETA(DisplayName = "Completed"),
    Interrupted  UMETA(DisplayName = "Interrupted")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    Player      UMETA(DisplayName = "Player"),
    NPC         UMETA(DisplayName = "NPC"),
    Narrator    UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity   UMETA(DisplayName = "Proximity"),
    QuestStart  UMETA(DisplayName = "QuestStart"),
    QuestEnd    UMETA(DisplayName = "QuestEnd"),
    Interaction UMETA(DisplayName = "Interaction"),
    Automatic   UMETA(DisplayName = "Automatic")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Supabase audio URL for this line
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // Duration in seconds (for auto-advance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    // If true, waits for player input before advancing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bWaitForInput;

    FNarr_DialogueLine()
        : LineID(NAME_None)
        , Speaker(ENarr_SpeakerRole::NPC)
        , SpeakerName(TEXT("Unknown"))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
        , bWaitForInput(true)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ChoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceText;

    // Which node this choice leads to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NextNodeID;

    // Optional: quest objective to complete on selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedQuestObjectiveID;

    FNarr_DialogueChoice()
        : ChoiceID(NAME_None)
        , ChoiceText(TEXT(""))
        , NextNodeID(NAME_None)
        , LinkedQuestObjectiveID(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueChoice> Choices;

    // If empty choices, auto-advance to this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName AutoNextNodeID;

    // Quest to activate when this node is reached
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ActivatesQuestID;

    FNarr_DialogueNode()
        : NodeID(NAME_None)
        , AutoNextNodeID(NAME_None)
        , ActivatesQuestID(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName EntryNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float TriggerRadius;

    // Linked quest ID (from QuestManager)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedQuestID;

    FNarr_DialogueTree()
        : TreeID(NAME_None)
        , NPCName(TEXT("Unknown"))
        , EntryNodeID(NAME_None)
        , TriggerType(ENarr_DialogueTrigger::Interaction)
        , TriggerLocation(FVector::ZeroVector)
        , TriggerRadius(300.0f)
        , LinkedQuestID(NAME_None)
    {}
};

// ─── UDialogueManager ─────────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UDialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Registration ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDefaultDialogueTrees();

    // ── Playback ──────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(FName TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    // ── Queries ───────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentSpeakerName() const;

    // ── Proximity check (called each tick by GameMode) ─────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckProximityTriggers(FVector PlayerLocation);

    // ── Lore text ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCraftingLore(FName RecipeID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCraftingLore(FName RecipeID, const FString& LoreText);

private:
    UPROPERTY()
    TArray<FNarr_DialogueTree> RegisteredTrees;

    UPROPERTY()
    TMap<FName, FString> CraftingLoreMap;

    // Active state
    FName ActiveTreeID;
    FName ActiveNodeID;
    int32 ActiveLineIndex;
    ENarr_DialogueState DialogueState;

    // Helpers
    FNarr_DialogueNode* FindNode(FName TreeID, FName NodeID);
    const FNarr_DialogueNode* FindNodeConst(FName TreeID, FName NodeID) const;
    void AdvanceToNode(FName NodeID);
    void TriggerQuestActivation(FName QuestID);
};
