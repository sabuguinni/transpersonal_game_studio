// NarrativeDialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue tree manager: phase-gated NPC lines, voice cue references, quest integration
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NarrativeDialogueManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialoguePhase : uint8
{
    NotStarted       UMETA(DisplayName = "Not Started"),
    Introduction     UMETA(DisplayName = "Introduction"),
    QuestBriefing    UMETA(DisplayName = "Quest Briefing"),
    MidJourney       UMETA(DisplayName = "Mid Journey"),
    CrisisPoint      UMETA(DisplayName = "Crisis Point"),
    Resolution       UMETA(DisplayName = "Resolution"),
    Epilogue         UMETA(DisplayName = "Epilogue")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder            UMETA(DisplayName = "Elder"),
    Scout            UMETA(DisplayName = "Scout"),
    Hunter           UMETA(DisplayName = "Hunter"),
    Crafter          UMETA(DisplayName = "Crafter"),
    TribalLeader     UMETA(DisplayName = "Tribal Leader"),
    Survivor         UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    OnProximity      UMETA(DisplayName = "On Proximity"),
    OnQuestPhase     UMETA(DisplayName = "On Quest Phase"),
    OnPlayerAction   UMETA(DisplayName = "On Player Action"),
    OnTimedEvent     UMETA(DisplayName = "On Timed Event"),
    OnDangerLevel    UMETA(DisplayName = "On Danger Level"),
    OnFirstMeeting   UMETA(DisplayName = "On First Meeting")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCRole SpeakerRole = ENarr_NPCRole::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTrigger TriggerType = ENarr_DialogueTrigger::OnProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialoguePhase RequiredPhase = ENarr_DialoguePhase::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float CooldownSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPlayedThisSession = false;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_NPCRole OwnerRole = ENarr_NPCRole::Elder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    int32 CurrentLineIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bIsActive = false;
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnDialogueLineTriggered,
    FName, TreeID, int32, LineIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueTreeCompleted,
    FName, TreeID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnPhaseGateUnlocked,
    ENarr_DialoguePhase, NewPhase, FName, TreeID);

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeDialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeDialogueManager();

    // ── Delegates ──
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineTriggered OnDialogueLineTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueTreeCompleted OnDialogueTreeCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnPhaseGateUnlocked OnPhaseGateUnlocked;

    // ── State ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|State")
    ENarr_DialoguePhase CurrentPhase = ENarr_DialoguePhase::NotStarted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Trees")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float ProximityTriggerRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bAutoAdvancePhase = true;

    // ── Methods ──
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void RegisterDialogueTree(const FNarr_DialogueTree& Tree);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool TriggerDialogueLine(FName TreeID, int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogueTree(FName TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SetNarrativePhase(ENarr_DialoguePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerPhaseGatedLines(ENarr_DialoguePhase Phase);

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine(FName TreeID) const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsTreeComplete(FName TreeID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void ResetTree(FName TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void LoadMigrationQuestDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void LoadCampDefenseDialogue();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    FNarr_DialogueTree* FindTree(FName TreeID);
    const FNarr_DialogueTree* FindTreeConst(FName TreeID) const;

    UPROPERTY()
    TMap<FName, float> LastTriggerTimestamps;

    float AccumulatedTime = 0.0f;
};
