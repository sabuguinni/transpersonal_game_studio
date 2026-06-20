// DialogueManager.h
// Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260620_005
// WorldSubsystem managing NPC dialogue trees, conversation state, and quest-linked dialogue.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueManager.generated.h"

// ============================================================
// Enums — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_Speaker : uint8
{
    Player  UMETA(DisplayName = "Player"),
    NPC     UMETA(DisplayName = "NPC"),
    Ambient UMETA(DisplayName = "Ambient Narration")
};

// ============================================================
// Structs — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bRequiresCondition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionTag = NAME_None;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_Speaker SpeakerTag = ENarr_Speaker::NPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<int32> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsEndNode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bTriggersQuestUpdate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName QuestUpdateID = NAME_None;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LinkedQuestID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> Nodes;
};

// ============================================================
// Delegate declarations
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueNodeReached, const FNarr_DialogueNode&, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnConversationEnded, FName, TreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnQuestDialogueTrigger, FName, QuestID);

// ============================================================
// UNarr_DialogueManager — WorldSubsystem
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // --- Subsystem lifecycle ---
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Dialogue control ---
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartConversation(FName TreeID, AActor* InstigatorActor);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceConversation(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndConversation();

    // --- Query ---
    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool HasDialogueTree(FName TreeID) const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FNarr_DialogueTree GetDialogueTree(FName TreeID) const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    TArray<FName> GetAllTreeIDs() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsConversationActive() const { return bConversationActive; }

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnDialogueNodeReached OnDialogueNodeReached;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnConversationEnded OnConversationEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Dialogue")
    FNarr_OnQuestDialogueTrigger OnQuestDialogueTrigger;

private:

    void RegisterDefaultDialogueTrees();

    UPROPERTY()
    TMap<FName, FNarr_DialogueTree> DialogueTrees;

    TOptional<FNarr_DialogueTree> ActiveConversation;

    UPROPERTY()
    AActor* ConversationInstigator = nullptr;

    int32 CurrentNodeIndex = 0;
    bool bConversationActive = false;

    FTimerHandle EndConversationTimer;
};
