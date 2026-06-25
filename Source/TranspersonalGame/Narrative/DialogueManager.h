#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENarr_DialogueState — lifecycle of a single dialogue exchange
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting for Player"),
    Completed   UMETA(DisplayName = "Completed"),
    Locked      UMETA(DisplayName = "Locked - Prereq not met")
};

// ─────────────────────────────────────────────────────────────────────────────
// ENarr_NPCRole — which quest giver / NPC archetype
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Scout       UMETA(DisplayName = "Scout"),
    Generic     UMETA(DisplayName = "Generic NPC")
};

// ─────────────────────────────────────────────────────────────────────────────
// ENarr_DialogueTrigger — when this line fires
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    QuestBriefing   UMETA(DisplayName = "Quest Briefing"),
    QuestCheckIn    UMETA(DisplayName = "Mid-Quest Check-In"),
    QuestComplete   UMETA(DisplayName = "Quest Completion"),
    QuestFailed     UMETA(DisplayName = "Quest Failed"),
    Ambient         UMETA(DisplayName = "Ambient / Idle"),
    Panic           UMETA(DisplayName = "Panic / Flee")
};

// ─────────────────────────────────────────────────────────────────────────────
// FNarr_DialogueLine — a single voiced line with metadata
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCRole SpeakerRole = ENarr_NPCRole::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTrigger Trigger = ENarr_DialogueTrigger::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LineText;

    // Quest ID this line belongs to (matches QuestManager quest IDs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString LinkedQuestID;

    // URL to the ElevenLabs-generated audio file in Supabase Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    // Duration in seconds (for subtitle timing)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DurationSeconds = 5.0f;

    // Has this line been played already?
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bPlayed = false;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , SpeakerName(TEXT(""))
        , SpeakerRole(ENarr_NPCRole::Generic)
        , Trigger(ENarr_DialogueTrigger::Ambient)
        , LineText(TEXT(""))
        , LinkedQuestID(TEXT(""))
        , AudioURL(TEXT(""))
        , DurationSeconds(5.0f)
        , bPlayed(false)
    {}
};

// ─────────────────────────────────────────────────────────────────────────────
// FNarr_NPCDialogueSet — all lines for one NPC
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FNarr_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    FString NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    ENarr_NPCRole Role = ENarr_NPCRole::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|NPC")
    TArray<FNarr_DialogueLine> Lines;

    FNarr_NPCDialogueSet()
        : NPCID(TEXT(""))
        , Role(ENarr_NPCRole::Generic)
    {}
};

// ─────────────────────────────────────────────────────────────────────────────
// ADialogueManager — world actor that owns all NPC dialogue data
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ADialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    // ── All NPC dialogue sets registered in this level ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_NPCDialogueSet> NPCDialogueSets;

    // ── Active dialogue state ──
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    ENarr_DialogueState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    FNarr_DialogueLine ActiveLine;

    // ── Public API ──

    // Trigger a specific line by NPC ID + trigger type
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool TriggerDialogue(const FString& NPCID, ENarr_DialogueTrigger TriggerType);

    // Called by QuestManager when a quest starts
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void OnQuestStarted(const FString& QuestID);

    // Called by QuestManager when a quest completes
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void OnQuestCompleted(const FString& QuestID);

    // Called by QuestManager when a quest fails
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void OnQuestFailed(const FString& QuestID);

    // Called by CrowdSimulation flee event (Agent #13 integration)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void OnCrowdPanic(float FearLevel);

    // Get all lines for a given quest ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> GetLinesForQuest(const FString& QuestID) const;

    // Mark a line as played
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void MarkLinePlayed(const FString& LineID);

    // Reset all played flags (e.g. on new game)
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetAllDialogue();

private:
    void RegisterElderDialogue();
    void RegisterHunterDialogue();
    void RegisterGathererDialogue();
    void RegisterScoutDialogue();

    FNarr_NPCDialogueSet* FindNPCSet(const FString& NPCID);
    FNarr_DialogueLine* FindLine(const FString& NPCID, ENarr_DialogueTrigger TriggerType);
};
