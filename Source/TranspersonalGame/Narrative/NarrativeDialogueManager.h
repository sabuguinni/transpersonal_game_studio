#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeDialogueManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting For Player"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    HunterLeader    UMETA(DisplayName = "Hunter Leader"),
    Tracker         UMETA(DisplayName = "Tracker"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    DefenseLeader   UMETA(DisplayName = "Defense Leader"),
    Generic         UMETA(DisplayName = "Generic NPC")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerResponse;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , SpeakerName(TEXT(""))
        , DialogueText(TEXT(""))
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
        , bRequiresPlayerResponse(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_PlayerChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bEndsDialogue;

    FNarr_PlayerChoice()
        : ChoiceText(TEXT(""))
        , NextNodeID(TEXT(""))
        , bEndsDialogue(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_PlayerChoice> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsEntryNode;

    FNarr_DialogueNode()
        : NodeID(TEXT(""))
        , LinkedQuestID(TEXT(""))
        , bIsEntryNode(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString OwnerNPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , OwnerNPCName(TEXT(""))
        , NPCRole(ENarr_NPCRole::Generic)
        , State(ENarr_DialogueState::Idle)
    {}
};

// ─── Manager Actor ────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    // ── Active dialogue state ─────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueTree ActiveDialogueTree;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    // ── All registered dialogue trees ────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> RegisteredTrees;

    // ── Lifecycle ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue(int32 PlayerChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueNode GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    // ── Registration ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueTree GetDialogueTree(const FString& TreeID) const;

    // ── Quest-linked dialogue ─────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartQuestDialogue(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetDialogueTreesForQuest(const FString& QuestID) const;

private:
    void InitializeDefaultDialogueTrees();
    FNarr_DialogueTree BuildHunterLeaderTree();
    FNarr_DialogueTree BuildTrackerTree();
    FNarr_DialogueTree BuildElderTree();
    FNarr_DialogueTree BuildCrafterTree();
    FNarr_DialogueTree BuildDefenseLeaderTree();
    FNarr_DialogueTree BuildMigrationScoutTree();

    FNarr_DialogueNode* FindNode(FNarr_DialogueTree& Tree, const FString& NodeID);
};
