#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueSystem.generated.h"

// ============================================================
// Enums — Narr_ prefix (unique across project)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    HunterScout     UMETA(DisplayName = "Hunter Scout"),
    CraftMaster     UMETA(DisplayName = "Craft Master"),
    ScoutTracker    UMETA(DisplayName = "Scout Tracker"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Player          UMETA(DisplayName = "Player")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    Always              UMETA(DisplayName = "Always"),
    HasItem             UMETA(DisplayName = "Has Item"),
    QuestActive         UMETA(DisplayName = "Quest Active"),
    QuestComplete       UMETA(DisplayName = "Quest Complete"),
    DayTime             UMETA(DisplayName = "Day Time"),
    NightTime           UMETA(DisplayName = "Night Time"),
    PlayerNearDanger    UMETA(DisplayName = "Player Near Danger"),
    FirstMeeting        UMETA(DisplayName = "First Meeting")
};

UENUM(BlueprintType)
enum class ENarr_DialogueOutcome : uint8
{
    None            UMETA(DisplayName = "None"),
    StartQuest      UMETA(DisplayName = "Start Quest"),
    CompleteQuest   UMETA(DisplayName = "Complete Quest"),
    GiveItem        UMETA(DisplayName = "Give Item"),
    OpenTrade       UMETA(DisplayName = "Open Trade"),
    JoinTribe       UMETA(DisplayName = "Join Tribe"),
    RevealLocation  UMETA(DisplayName = "Reveal Location")
};

// ============================================================
// Structs — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bWaitForInput;

    FNarr_DialogueLine()
        : LineID(NAME_None)
        , Speaker(ENarr_SpeakerRole::TribalElder)
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
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueCondition Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueOutcome Outcome;

    FNarr_DialogueChoice()
        : ChoiceID(NAME_None)
        , NextNodeID(NAME_None)
        , Condition(ENarr_DialogueCondition::Always)
        , Outcome(ENarr_DialogueOutcome::None)
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_SpeakerRole OwnerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName RootNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueNode> Nodes;

    FNarr_DialogueTree()
        : TreeID(NAME_None)
        , OwnerRole(ENarr_SpeakerRole::TribalElder)
        , RootNodeID(NAME_None)
    {}
};

// ============================================================
// Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueStarted, FName, TreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, TreeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnLineDisplayed, FName, NodeID, int32, LineIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnChoiceMade, FName, ChoiceID, ENarr_DialogueOutcome, Outcome);

// ============================================================
// UDialogueSystem — ActorComponent
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueSystem();

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnLineDisplayed OnLineDisplayed;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnChoiceMade OnChoiceMade;

    // --- Data ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName ActiveTreeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FName ActiveNodeID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 ActiveLineIndex;

    // --- API ---
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(FName TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool MakeChoice(FName ChoiceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasActiveDialogue() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeElderDialogueTrees();

protected:
    virtual void BeginPlay() override;

private:
    const FNarr_DialogueNode* FindNode(const FNarr_DialogueTree* Tree, FName NodeID) const;
    const FNarr_DialogueTree* FindTree(FName TreeID) const;
    bool EvaluateCondition(ENarr_DialogueCondition Condition) const;
};
