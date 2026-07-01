
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueTree.generated.h"

// ============================================================
// ENarr_DialogueConditionType — what triggers a dialogue node
// ============================================================
UENUM(BlueprintType)
enum class ENarr_DialogueConditionType : uint8
{
    None            UMETA(DisplayName = "None"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    HasItem         UMETA(DisplayName = "Has Item"),
    FirstMeeting    UMETA(DisplayName = "First Meeting"),
    TimeOfDay       UMETA(DisplayName = "Time Of Day"),
    DinosaurNearby  UMETA(DisplayName = "Dinosaur Nearby")
};

// ============================================================
// ENarr_NPCRole — which NPC archetype owns this dialogue
// ============================================================
UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    CampLeader  UMETA(DisplayName = "Camp Leader"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Artisan     UMETA(DisplayName = "Artisan")
};

// ============================================================
// FNarr_DialogueChoice — a single player response option
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName UnlocksQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bEndsConversation = false;
};

// ============================================================
// FNarr_DialogueNode — a single NPC speech node
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NPCLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueConditionType Condition = ENarr_DialogueConditionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName ConditionQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsEntryNode = false;
};

// ============================================================
// UDialogueTree — ActorComponent managing NPC dialogue trees
// ============================================================
UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDialogueTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueTree();

    virtual void BeginPlay() override;

    // --- NPC identity ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NPCID;

    // --- Dialogue nodes ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName CurrentNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bConversationActive = false;

    // --- Active quests known to this NPC ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FName> KnownQuestIDs;

    // --- Delegates ---
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeReached, FName, NodeID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestUnlocked, FName, QuestID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConversationEnded);

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueNodeReached OnDialogueNodeReached;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnQuestUnlocked OnQuestUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnConversationEnded OnConversationEnded;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartConversation();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueNode GetCurrentNode() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsConditionMet(const FNarr_DialogueNode& Node) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDefaultDialogue_Elder();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDefaultDialogue_Scout();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDefaultDialogue_CampLeader();

private:
    FNarr_DialogueNode* FindNode(FName NodeID);
    TSet<FName> CompletedQuestIDs;
};
