#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystemManager.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueSystemManager
// Wires NPC dialogue trees to QuestSystemManager quest IDs.
// Characters: Elder Kael, Hunter Chief Brak, Scout Mira
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    HunterChief     UMETA(DisplayName = "HunterChief"),
    Scout           UMETA(DisplayName = "Scout"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    FNarr_DialogueLine()
        : SpeakerName(TEXT(""))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
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

    /** Quest ID to activate when this node completes (empty = no quest trigger) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TriggersQuestID;

    /** Next node ID after this one (empty = end conversation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextNodeID;

    FNarr_DialogueNode()
        : NodeID(TEXT(""))
        , TriggersQuestID(TEXT(""))
        , NextNodeID(TEXT(""))
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
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , NPCName(TEXT(""))
        , NPCRole(ENarr_NPCRole::Generic)
        , RootNodeID(TEXT(""))
        , State(ENarr_DialogueState::Idle)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueLineDelegate, FString, SpeakerName, FString, LineText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestTriggeredDelegate, FString, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEndedDelegate, FString, TreeID);

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    // ---- Dialogue Trees ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString ActiveTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString ActiveNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bDialogueActive;

    // ---- Events ----
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineDelegate OnDialogueLine;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnQuestTriggeredDelegate OnQuestTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEndedDelegate OnDialogueEnded;

    // ---- Functions ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueTree GetDialogueTree(const FString& TreeID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

private:
    void InitializeDefaultDialogueTrees();
    FNarr_DialogueNode* FindNode(FNarr_DialogueTree& Tree, const FString& NodeID);
    int32 ActiveLineIndex;
};
