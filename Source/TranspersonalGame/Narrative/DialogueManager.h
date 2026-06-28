#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueSystemTypes.h"
#include "DialogueManager.generated.h"

// ============================================================
// DIALOGUE MANAGER — Agent #15 Narrative & Dialogue
// Manages all NPC dialogue trees, proximity detection,
// and quest integration for MinPlayableMap NPCs
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarr_DialogueLineSpoken, const FNarr_DialogueEvent&, DialogueEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarr_DialogueStateChanged, FName, NPCID, ENarr_DialogueState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarr_QuestOfferTriggered, FName, NPCID, FName, QuestID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // -------------------------------------------------------
    // DIALOGUE REGISTRY
    // -------------------------------------------------------

    // All registered NPC dialogue trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Registry")
    TArray<FNarr_DialogueTree> RegisteredDialogueTrees;

    // Currently active dialogue (one at a time)
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FName ActiveNPCID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FName ActiveNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialogueState ActiveDialogueState = ENarr_DialogueState::Idle;

    // Is a dialogue currently in progress?
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bDialogueActive = false;

    // -------------------------------------------------------
    // PROXIMITY DETECTION
    // -------------------------------------------------------

    // How often to check player proximity to NPCs (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Settings")
    float ProximityCheckInterval = 0.3f;

    // -------------------------------------------------------
    // DELEGATES
    // -------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnNarr_DialogueLineSpoken OnDialogueLineSpoken;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnNarr_DialogueStateChanged OnDialogueStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnNarr_QuestOfferTriggered OnQuestOfferTriggered;

    // -------------------------------------------------------
    // PUBLIC API
    // -------------------------------------------------------

    // Register a dialogue tree for an NPC
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTree(const FNarr_DialogueTree& DialogueTree);

    // Start dialogue with a specific NPC
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(FName NPCID);

    // Advance dialogue with a player response choice (0-based index)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue(int32 ResponseIndex);

    // End the current dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Get the current active node for UI display
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetActiveNode(FNarr_DialogueNode& OutNode) const;

    // Get the NPC name for the active dialogue
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetActiveNPCName() const;

    // Notify that a quest state changed (updates NPC dialogue state)
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void NotifyQuestStateChanged(FName QuestID, FName NewStateName);

    // Initialize built-in dialogue trees for MinPlayableMap NPCs
    UFUNCTION(CallInEditor, Category = "Narrative")
    void InitializeBuiltInDialogueTrees();

    // Validate all registered dialogue trees (checks node references)
    UFUNCTION(CallInEditor, Category = "Narrative")
    void ValidateDialogueTrees();

private:
    // Internal: find a dialogue tree by NPC ID
    FNarr_DialogueTree* FindDialogueTree(FName NPCID);
    const FNarr_DialogueTree* FindDialogueTreeConst(FName NPCID) const;

    // Internal: find a node within a tree by node ID
    const FNarr_DialogueNode* FindNode(const FNarr_DialogueTree& Tree, FName NodeID) const;

    // Internal: evaluate whether a dialogue condition is met
    bool EvaluateCondition(ENarr_DialogueCondition Condition, FName QuestID) const;

    // Internal: broadcast a dialogue event
    void BroadcastDialogueEvent(const FNarr_DialogueNode& Node, ENarr_DialogueState NewState);

    // Proximity check timer
    float ProximityCheckTimer = 0.0f;

    // Quest state cache: QuestID -> state name
    TMap<FName, FName> QuestStateCache;

    // -------------------------------------------------------
    // BUILT-IN DIALOGUE DATA
    // -------------------------------------------------------

    // TribalElder — "Survive the Stampede" quest giver
    FNarr_DialogueTree BuildTribalElderDialogue() const;

    // Scout — "Ghost of the Ridge" stealth quest giver
    FNarr_DialogueTree BuildScoutDialogue() const;

    // Watcher — "The Watcher" observation quest giver
    FNarr_DialogueTree BuildWatcherDialogue() const;

    // Villager — "Missing Brother" side quest
    FNarr_DialogueTree BuildVillagerDialogue() const;
};
