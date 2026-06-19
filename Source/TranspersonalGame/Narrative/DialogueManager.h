#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueManager.generated.h"

// -----------------------------------------------------------------------
// Narrative & Dialogue Agent #15 — DialogueManager
// Dialogue tree system for NPC conversations and quest integration
// -----------------------------------------------------------------------

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "WaitingForChoice"),
    Completed   UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    Player      UMETA(DisplayName = "Player"),
    NPC         UMETA(DisplayName = "NPC"),
    Narrator    UMETA(DisplayName = "Narrator")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestIDToActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bEndsDialogue;

    FNarr_DialogueChoice()
        : ChoiceText(TEXT(""))
        , NextNodeID(TEXT(""))
        , QuestIDToActivate(TEXT(""))
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
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole SpeakerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioCueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsEndNode;

    FNarr_DialogueNode()
        : NodeID(TEXT(""))
        , SpeakerName(TEXT(""))
        , SpeakerRole(ENarr_SpeakerRole::NPC)
        , DialogueText(TEXT(""))
        , AudioCueID(TEXT(""))
        , bIsEndNode(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredQuestID;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , NPCName(TEXT(""))
        , RootNodeID(TEXT(""))
        , RequiredQuestID(TEXT(""))
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarr_DialogueNodeChanged, FString, NodeID, FNarr_DialogueNode, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarr_DialogueEnded, FString, TreeID);

UCLASS()
class TRANSPERSONALGAME_API UDialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Start a dialogue tree by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& TreeID);

    // Player selects a choice (index into current node's Choices array)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool SelectChoice(int32 ChoiceIndex);

    // Advance to a specific node
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceToNode(const FString& NodeID);

    // End current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get current active node
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueNode GetCurrentNode() const;

    // Get current dialogue state
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_DialogueState GetDialogueState() const { return CurrentState; }

    // Register a dialogue tree
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    // Check if a tree is registered
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasDialogueTree(const FString& TreeID) const;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarr_DialogueNodeChanged OnDialogueNodeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarr_DialogueEnded OnDialogueEnded;

private:
    void RegisterDefaultDialogueTrees();
    FNarr_DialogueTree BuildTribeElderTree();
    FNarr_DialogueTree BuildTribeScoutTree();
    const FNarr_DialogueNode* FindNode(const FNarr_DialogueTree& Tree, const FString& NodeID) const;

    UPROPERTY()
    TMap<FString, FNarr_DialogueTree> DialogueTrees;

    ENarr_DialogueState CurrentState;
    FString ActiveTreeID;
    FString CurrentNodeID;
};
