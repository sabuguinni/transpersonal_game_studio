#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueManager.generated.h"

// ============================================================
// Narrative & Dialogue Agent #15 — DialogueManager
// Dialogue system for NPC conversations, quest dialogue,
// and tribal elder interactions in MinPlayableMap.
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    WaitingInput    UMETA(DisplayName = "WaitingInput"),
    Completed       UMETA(DisplayName = "Completed"),
    Interrupted     UMETA(DisplayName = "Interrupted")
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    QuestGive       UMETA(DisplayName = "QuestGive"),
    QuestCheckIn    UMETA(DisplayName = "QuestCheckIn"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    QuestFailed     UMETA(DisplayName = "QuestFailed"),
    Greeting        UMETA(DisplayName = "Greeting"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Warning         UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class ENarr_NPCIdentity : uint8
{
    TribalElder     UMETA(DisplayName = "TribalElder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Survivor        UMETA(DisplayName = "Survivor")
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
    ENarr_NPCIdentity SpeakerIdentity = ENarr_NPCIdentity::TribalElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bRequiresPlayerInput = false;

    FNarr_DialogueLine() {}
    FNarr_DialogueLine(const FString& Speaker, const FString& Text, float Duration = 4.0f)
        : SpeakerName(Speaker), LineText(Text), DisplayDuration(Duration) {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueContext Context = ENarr_DialogueContext::Greeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed = false;

    FNarr_DialogueSequence() {}
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCIdentity NPCIdentity = ENarr_NPCIdentity::TribalElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NPCLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueSequence> Sequences;

    FNarr_NPCDialogueSet() {}
};

/**
 * UDialogueManager — World Subsystem
 * Manages all NPC dialogue sequences, quest-linked conversations,
 * and tribal elder interactions. Queries QuestManager state to
 * branch dialogue based on active/completed quests.
 */
UCLASS()
class TRANSPERSONALGAME_API UDialogueManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDialogueManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Core Dialogue API ---

    /** Start a dialogue sequence by ID */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartDialogue(const FString& SequenceID);

    /** Advance to next line in active sequence */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool AdvanceDialogue();

    /** End current dialogue immediately */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    /** Get current active dialogue line */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    /** Get current dialogue state */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    ENarr_DialogueState GetDialogueState() const { return CurrentState; }

    /** Is any dialogue currently active? */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const { return CurrentState == ENarr_DialogueState::Active || CurrentState == ENarr_DialogueState::WaitingInput; }

    // --- Quest-Linked Dialogue ---

    /** Get appropriate dialogue sequence ID for NPC + quest context */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FString GetQuestDialogueID(ENarr_NPCIdentity NPC, const FString& QuestID, ENarr_DialogueContext Context) const;

    /** Trigger elder dialogue based on current quest state */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerElderDialogue(const FString& QuestID);

    // --- Dialogue Library ---

    /** Get all sequences for a given NPC */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueSequence> GetNPCSequences(ENarr_NPCIdentity NPC) const;

    /** Mark a sequence as played */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void MarkSequencePlayed(const FString& SequenceID);

    /** Has sequence been played before? */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool HasSequenceBeenPlayed(const FString& SequenceID) const;

    // --- Ambient Dialogue ---

    /** Get random ambient line for NPC (for idle chatter) */
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetAmbientLine(ENarr_NPCIdentity NPC) const;

private:
    UPROPERTY()
    ENarr_DialogueState CurrentState;

    UPROPERTY()
    FString ActiveSequenceID;

    UPROPERTY()
    int32 CurrentLineIndex;

    UPROPERTY()
    TArray<FNarr_NPCDialogueSet> DialogueLibrary;

    UPROPERTY()
    TArray<FString> PlayedSequences;

    void BuildDialogueLibrary();
    void BuildElderDialogues(FNarr_NPCDialogueSet& ElderSet);
    void BuildHunterDialogues(FNarr_NPCDialogueSet& HunterSet);
    void BuildScoutDialogues(FNarr_NPCDialogueSet& ScoutSet);

    FNarr_DialogueSequence* FindSequence(const FString& SequenceID);
    const FNarr_DialogueSequence* FindSequenceConst(const FString& SequenceID) const;
};
