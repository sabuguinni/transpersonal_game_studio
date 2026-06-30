// NarrDialogueTree.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue tree system for quest NPCs — phase-conditional branches tied to EQuest_MigrationPhase
// PROD_CYCLE_AUTO_20260630_008

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrDialogueTree.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Player      UMETA(DisplayName = "Player"),
    Narrator    UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarr_DialogueCondition : uint8
{
    Always              UMETA(DisplayName = "Always"),
    PhaseLocateHerd     UMETA(DisplayName = "Phase: Locate Herd"),
    PhaseFollowHerd     UMETA(DisplayName = "Phase: Follow Herd"),
    PhaseReachWater     UMETA(DisplayName = "Phase: Reach Water"),
    PhaseStampede       UMETA(DisplayName = "Phase: Survive Stampede"),
    QuestComplete       UMETA(DisplayName = "Quest Complete"),
    QuestFailed         UMETA(DisplayName = "Quest Failed"),
    PlayerLowThirst     UMETA(DisplayName = "Player Low Thirst"),
    PlayerLowHealth     UMETA(DisplayName = "Player Low Health"),
    NightTime           UMETA(DisplayName = "Night Time")
};

UENUM(BlueprintType)
enum class ENarr_DialogueResponseType : uint8
{
    Acknowledge         UMETA(DisplayName = "Acknowledge"),
    AskForDetails       UMETA(DisplayName = "Ask For Details"),
    RefuseQuest         UMETA(DisplayName = "Refuse Quest"),
    AcceptQuest         UMETA(DisplayName = "Accept Quest"),
    AskAboutDanger      UMETA(DisplayName = "Ask About Danger"),
    AskAboutWater       UMETA(DisplayName = "Ask About Water"),
    Farewell            UMETA(DisplayName = "Farewell")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — global scope (RULE 1)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueCondition Condition = ENarr_DialogueCondition::Always;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bCanBeSkipped = true;

    FNarr_DialogueLine() {}

    FNarr_DialogueLine(ENarr_DialogueSpeaker InSpeaker, const FString& InText, ENarr_DialogueCondition InCondition = ENarr_DialogueCondition::Always)
        : Speaker(InSpeaker), LineText(InText), Condition(InCondition) {}
};

USTRUCT(BlueprintType)
struct FNarr_PlayerResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString ResponseText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueResponseType ResponseType = ENarr_DialogueResponseType::Acknowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NextNodeIndex = -1;

    FNarr_PlayerResponse() {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 NodeIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_PlayerResponse> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsTerminalNode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bTriggersQuestStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString NodeLabel;

    FNarr_DialogueNode() {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 CurrentNodeIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString CurrentSpeakerName;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FString CurrentLineText;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    TArray<FString> AvailableResponses;

    FNarr_DialogueState() {}
};

// ─────────────────────────────────────────────────────────────────────────────
// DELEGATES
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueLineDisplayed, const FNarr_DialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueNodeComplete, int32, NodeIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNarr_OnDialogueTreeComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNarr_OnQuestStartTriggered);

// ─────────────────────────────────────────────────────────────────────────────
// ANarrDialogueTree — Actor component managing NPC dialogue trees
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ANarrDialogueTree : public AActor
{
    GENERATED_BODY()

public:
    ANarrDialogueTree();

    // ── Dialogue Data ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueNode> DialogueNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker NPCSpeakerType = ENarr_DialogueSpeaker::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float InteractionRadius = 300.0f;

    // ── Runtime State ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FNarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bQuestStarted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    int32 TimesInteracted = 0;

    // ── Delegates ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineDisplayed OnDialogueLineDisplayed;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueNodeComplete OnDialogueNodeComplete;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueTreeComplete OnDialogueTreeComplete;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnQuestStartTriggered OnQuestStartTriggered;

    // ── Public Interface ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(ENarr_DialogueCondition CurrentPhaseCondition);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SelectResponse(int32 ResponseIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsPlayerInRange(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> GetLinesForCondition(ENarr_DialogueCondition Condition) const;

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DEBUG_PrintDialogueTree();

    UFUNCTION(CallInEditor, Category = "Narrative|Debug")
    void DEBUG_StartElderDialogue();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void BuildElderDialogueTree();
    void BuildScoutDialogueTree();
    void DisplayCurrentLine();
    bool IsConditionMet(ENarr_DialogueCondition Condition) const;

    ENarr_DialogueCondition CachedPhaseCondition = ENarr_DialogueCondition::Always;
    float LineDisplayTimer = 0.0f;
    bool bAutoAdvance = false;
};
