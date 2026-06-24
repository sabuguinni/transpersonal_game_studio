// DialogueManager.h — Agent #15 Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260624_008
// Manages dialogue trees, NPC voice lines, and narrative trigger events.
// Integrates with QuestManager (Agent #14) for quest-linked dialogue.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "DialogueManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting for Player"),
    Complete    UMETA(DisplayName = "Complete"),
    Locked      UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    Narrator        UMETA(DisplayName = "Narrator"),
    PlayerCharacter UMETA(DisplayName = "Player Character"),
    NPC_Generic     UMETA(DisplayName = "Generic NPC")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    ProximityEnter  UMETA(DisplayName = "Proximity Enter"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    ItemPickup      UMETA(DisplayName = "Item Pickup"),
    FirstEncounter  UMETA(DisplayName = "First Encounter")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerType Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText DialogueText;

    /** ElevenLabs TTS audio URL for this line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    /** Duration in seconds (estimated from TTS) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float Duration;

    /** If true, pauses gameplay during this line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPausesGameplay;

    FNarr_DialogueLine()
        : LineID(NAME_None)
        , Speaker(ENarr_SpeakerType::NPC_Generic)
        , AudioURL(TEXT(""))
        , Duration(3.0f)
        , bPausesGameplay(false)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTriggerType TriggerType;

    /** Quest ID this dialogue is linked to (optional) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName LinkedQuestID;

    /** If true, this tree can only play once per session */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPlayOnce;

    /** Whether this tree has already been played */
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bHasPlayed;

    FNarr_DialogueTree()
        : TreeID(NAME_None)
        , TriggerType(ENarr_DialogueTriggerType::ProximityEnter)
        , LinkedQuestID(NAME_None)
        , bPlayOnce(true)
        , bHasPlayed(false)
    {}
};

// ─── DialogueManager Subsystem ───────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Dialogue Playback ──────────────────────────────────────────────────

    /** Start a dialogue tree by ID */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogueTree(FName TreeID);

    /** Advance to the next line in the active dialogue */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    /** Skip the current dialogue tree */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SkipDialogue();

    /** Returns true if a dialogue is currently active */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const;

    /** Get the current dialogue line being played */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    // ── Quest-Linked Dialogue ──────────────────────────────────────────────

    /** Trigger dialogue linked to a quest event */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerQuestDialogue(FName QuestID, ENarr_DialogueTriggerType TriggerType);

    /** Register a dialogue tree in the database */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    // ── Delegates ─────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, FName, TreeID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueLineChanged, FNarr_DialogueLine, Line);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, FName, TreeID);

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnDialogueLineChanged OnDialogueLineChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnDialogueCompleted OnDialogueCompleted;

private:
    /** All registered dialogue trees */
    UPROPERTY()
    TMap<FName, FNarr_DialogueTree> DialogueDatabase;

    /** Currently active tree */
    UPROPERTY()
    FName ActiveTreeID;

    /** Current line index within active tree */
    int32 CurrentLineIndex;

    /** Current state of the dialogue system */
    ENarr_DialogueState CurrentState;

    /** Build the default dialogue database from hardcoded survival content */
    void BuildDefaultDialogueDatabase();
};
