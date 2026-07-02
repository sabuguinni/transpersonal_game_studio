#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    Statement       UMETA(DisplayName = "Statement"),
    PlayerChoice    UMETA(DisplayName = "PlayerChoice"),
    QuestTrigger    UMETA(DisplayName = "QuestTrigger"),
    LoreUnlock      UMETA(DisplayName = "LoreUnlock"),
    Farewell        UMETA(DisplayName = "Farewell")
};

UENUM(BlueprintType)
enum class ENarr_NPCMood : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Urgent          UMETA(DisplayName = "Urgent"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Grateful        UMETA(DisplayName = "Grateful"),
    Hostile         UMETA(DisplayName = "Hostile")
};

UENUM(BlueprintType)
enum class ENarr_QuestStateContext : uint8
{
    NotStarted      UMETA(DisplayName = "NotStarted"),
    InProgress      UMETA(DisplayName = "InProgress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class ENarr_LoreCategory : uint8
{
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Tribe           UMETA(DisplayName = "Tribe"),
    Environment     UMETA(DisplayName = "Environment"),
    Survival        UMETA(DisplayName = "Survival"),
    History         UMETA(DisplayName = "History")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

/** A single node in a branching dialogue tree */
USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueNodeType NodeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    /** IDs of child nodes (player choices or auto-advance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FName> ChildNodeIDs;

    /** Quest name to activate when this node is reached (if QuestTrigger type) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedQuestName;

    /** Lore entry to unlock when this node is reached (if LoreUnlock type) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LinkedLoreEntryID;

    /** Required quest state context for this node to be available */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestStateContext RequiredQuestState;

    /** Audio URL for this line (ElevenLabs TTS) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    FNarr_DialogueNode()
        : NodeType(ENarr_DialogueNodeType::Statement)
        , RequiredQuestState(ENarr_QuestStateContext::NotStarted)
    {}
};

/** A complete dialogue tree for one NPC */
USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FName, FNarr_DialogueNode> Nodes;

    FNarr_DialogueTree() {}
};

/** A lore/journal entry unlocked through gameplay */
USTRUCT(BlueprintType)
struct FNarr_LoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName EntryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText BodyText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_LoreCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsUnlocked;

    FNarr_LoreEntry()
        : Category(ENarr_LoreCategory::Survival)
        , bIsUnlocked(false)
    {}
};

// ============================================================
// ANarr_DialogueManager — spawnable actor that owns all trees
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    /** All registered dialogue trees, keyed by TreeID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FName, FNarr_DialogueTree> DialogueTrees;

    /** All lore entries, keyed by EntryID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FName, FNarr_LoreEntry> LoreEntries;

    /** Currently active dialogue tree (empty = no dialogue active) */
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName ActiveTreeID;

    /** Currently active node within the active tree */
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName ActiveNodeID;

    /** Start a dialogue tree by ID */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(FName TreeID);

    /** Advance to a child node by ID (player choice or auto) */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceToNode(FName NodeID);

    /** End the current dialogue */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    /** Get the current active node data */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueNode GetCurrentNode() const;

    /** Unlock a lore entry by ID */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool UnlockLoreEntry(FName EntryID);

    /** Get all unlocked lore entries */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_LoreEntry> GetUnlockedLoreEntries() const;

    /** Register the ElderHunter dialogue tree (called at BeginPlay) */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterElderHunterDialogue();

    /** Register the Tracker dialogue tree (called at BeginPlay) */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterTrackerDialogue();

    /** Register all lore entries for the Stampede Escape quest */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterStampedeLore();

    /** Delegate broadcast when a new node becomes active */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueNodeActivated, FName, TreeID, FNarr_DialogueNode, Node);
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueNodeActivated OnDialogueNodeActivated;

    /** Delegate broadcast when a lore entry is unlocked */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoreEntryUnlocked, FNarr_LoreEntry, Entry);
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnLoreEntryUnlocked OnLoreEntryUnlocked;

protected:
    virtual void BeginPlay() override;

private:
    /** Helper: build and add a node to a tree */
    void AddNode(FNarr_DialogueTree& Tree, FName NodeID, ENarr_DialogueNodeType Type,
                 FText Speaker, FText Text, TArray<FName> Children,
                 FName QuestLink = NAME_None, FName LoreLink = NAME_None,
                 FString AudioURL = TEXT(""));
};

// ============================================================
// ANarr_NPCDialogueTrigger — proximity trigger on NPC actors
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_NPCDialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCDialogueTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBillboardComponent* Billboard;

    /** Which dialogue tree this NPC triggers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName DialogueTreeID;

    /** NPC display name shown in UI */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText NPCDisplayName;

    /** Reference to the dialogue manager in the level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ANarr_DialogueManager* DialogueManager;

    /** Radius within which player can interact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float InteractionRadius;

    /** Is the player currently in range? */
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bPlayerInRange;

    /** Trigger dialogue (called when player presses interact key) */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnterRange, FText, NPCName);
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnPlayerEnterRange OnPlayerEnterRange;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerExitRange);
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnPlayerExitRange OnPlayerExitRange;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                               bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    ANarr_NPCDialogueTrigger()
        : InteractionRadius(250.0f)
        , bPlayerInRange(false)
    {}
};
