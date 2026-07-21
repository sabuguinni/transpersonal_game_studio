#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCDialogueTree.generated.h"

// ============================================================
// NPCDialogueTree.h — Agent #15 Narrative & Dialogue
// Full dialogue tree for HerdTracker NPC with phase-gated
// branches tied to EQuest_MigrationPhase progression.
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialoguePhase : uint8
{
    Idle            UMETA(DisplayName = "Idle — Before Quest"),
    LocateHerd      UMETA(DisplayName = "Phase 1 — Locate Herd"),
    TrackMigration  UMETA(DisplayName = "Phase 2 — Track Migration"),
    AvoidPredators  UMETA(DisplayName = "Phase 3 — Avoid Predators"),
    ReachCrossing   UMETA(DisplayName = "Phase 4 — Reach Crossing"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    QuestFailed     UMETA(DisplayName = "Quest Failed")
};

UENUM(BlueprintType)
enum class ENarr_PlayerChoice : uint8
{
    AskAboutHerd        UMETA(DisplayName = "Ask about the herd"),
    AskAboutRaptors     UMETA(DisplayName = "Ask about the raptors"),
    AskAboutRoute       UMETA(DisplayName = "Ask about the route"),
    AskAboutWeather     UMETA(DisplayName = "Ask about the weather"),
    AcceptQuest         UMETA(DisplayName = "Accept quest"),
    DeclineQuest        UMETA(DisplayName = "Decline quest"),
    ReportProgress      UMETA(DisplayName = "Report progress"),
    None                UMETA(DisplayName = "No choice")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    // Audio asset path (populated from ElevenLabs TTS URLs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    // Which player choices are available after this line
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<ENarr_PlayerChoice> AvailableChoices;

    // Phase required to unlock this line (None = always available)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialoguePhase RequiredPhase = ENarr_DialoguePhase::Idle;

    FNarr_DialogueLine() {}
    FNarr_DialogueLine(const FString& Speaker, const FString& Text, ENarr_DialoguePhase Phase)
        : SpeakerName(Speaker), LineText(Text), RequiredPhase(Phase) {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_DialogueLine Line;

    // Maps player choice → next NodeID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<ENarr_PlayerChoice, FName> ChoiceBranches;

    // NodeID to jump to if no choice made (auto-advance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName DefaultNextNode = NAME_None;

    FNarr_DialogueNode() {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueNodeReached, FName, NodeID, FNarr_DialogueLine, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, LastNodeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnPlayerChoiceMade, ENarr_PlayerChoice, Choice, FName, NextNodeID);

UCLASS(ClassGroup = "Narrative", meta = (BlueprintSpawnableComponent), DisplayName = "NPC Dialogue Tree")
class TRANSPERSONALGAME_API UNPCDialogueTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDialogueTree();

    // ── Dialogue State ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bDialogueActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FName CurrentNodeID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    ENarr_DialoguePhase CurrentPhase = ENarr_DialoguePhase::Idle;

    // ── Dialogue Tree Data ──────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Tree")
    TMap<FName, FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Tree")
    FName EntryNodeID = FName("ROOT");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCName = TEXT("HerdTracker");

    // ── Delegates ───────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueNodeReached OnDialogueNodeReached;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnPlayerChoiceMade OnPlayerChoiceMade;

    // ── Public API ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MakeChoice(ENarr_PlayerChoice Choice);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceToNode(FName NodeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetQuestPhase(ENarr_DialoguePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<ENarr_PlayerChoice> GetAvailableChoices() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsDialogueActive() const { return bDialogueActive; }

    // Builds the HerdTracker NPC full dialogue tree at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void BuildHerdTrackerDialogueTree();

protected:
    virtual void BeginPlay() override;

private:
    void AddNode(FName NodeID, const FString& Speaker, const FString& Text,
                 ENarr_DialoguePhase Phase, FName DefaultNext = NAME_None);
    void AddBranch(FName FromNodeID, ENarr_PlayerChoice Choice, FName ToNodeID);
};
