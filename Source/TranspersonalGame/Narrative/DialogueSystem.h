#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Speaking    UMETA(DisplayName = "Speaking"),
    Waiting     UMETA(DisplayName = "Waiting for Player"),
    Finished    UMETA(DisplayName = "Finished")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Tracker     UMETA(DisplayName = "Tracker"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Narrator    UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_QuestArc : uint8
{
    None            UMETA(DisplayName = "None"),
    FirstTools      UMETA(DisplayName = "First Tools"),
    RaptorNest      UMETA(DisplayName = "Raptor Nest"),
    MammothHunt     UMETA(DisplayName = "Mammoth Hunt"),
    TribeLeader     UMETA(DisplayName = "Tribe Leader")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_QuestArc TriggerArc;

    FNarr_DialogueLine()
        : LineID(NAME_None)
        , Speaker(ENarr_SpeakerRole::Narrator)
        , DisplayDuration(4.0f)
        , TriggerArc(ENarr_QuestArc::None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bUnlocksQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_QuestArc UnlockedArc;

    FNarr_DialogueChoice()
        : NextNodeID(NAME_None)
        , bUnlocksQuest(false)
        , UnlockedArc(ENarr_QuestArc::None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_DialogueLine Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsEndNode;

    FNarr_DialogueNode()
        : NodeID(NAME_None)
        , bIsEndNode(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerRole OwnerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_QuestArc RequiredArc;

    FNarr_DialogueTree()
        : TreeID(NAME_None)
        , OwnerRole(ENarr_SpeakerRole::Narrator)
        , RootNodeID(NAME_None)
        , RequiredArc(ENarr_QuestArc::None)
    {}
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueStarted, FName, TreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueLineShown, FNarr_DialogueLine, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestArcUnlocked, ENarr_QuestArc, Arc);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNarr_OnDialogueEnded);

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    // ── Dialogue Trees ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    // ── Active State ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FName ActiveTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FName ActiveNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    TArray<ENarr_QuestArc> UnlockedArcs;

    // ── Delegates ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineShown OnDialogueLineShown;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnQuestArcUnlocked OnQuestArcUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    // ── Functions ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(FName TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueNode GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasUnlockedArc(ENarr_QuestArc Arc) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void UnlockArc(ENarr_QuestArc Arc);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    // ── Audio URLs (set from TTS generation) ─────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString ElderCraftedURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString TrackerRaptorURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString TrackerMammothURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString ElderReturnURL;

protected:
    virtual void BeginPlay() override;

private:
    FNarr_DialogueTree* FindTree(FName TreeID);
    FNarr_DialogueNode* FindNode(FNarr_DialogueTree* Tree, FName NodeID);
    void ShowNode(FNarr_DialogueNode* Node);
    void BuildDefaultTrees();
};
