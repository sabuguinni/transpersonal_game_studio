// DialogueManager.h
// Agent #15 — Narrative & Dialogue Agent
// PROD_CYCLE_AUTO_20260627_002
// Manages NPC dialogue trees, quest-linked conversations, and lore discovery

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Active          UMETA(DisplayName = "Active"),
    WaitingChoice   UMETA(DisplayName = "WaitingChoice"),
    Completed       UMETA(DisplayName = "Completed"),
    Locked          UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    Elder           UMETA(DisplayName = "Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    CampLeader      UMETA(DisplayName = "CampLeader"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Stranger        UMETA(DisplayName = "Stranger")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "QuestStart"),
    QuestComplete   UMETA(DisplayName = "QuestComplete"),
    QuestFail       UMETA(DisplayName = "QuestFail"),
    LoreDiscover    UMETA(DisplayName = "LoreDiscover"),
    PlayerDeath     UMETA(DisplayName = "PlayerDeath")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString NextLineID;

    FNarr_DialogueLine()
        : LineID(TEXT(""))
        , SpeakerName(TEXT(""))
        , DialogueText(FText::GetEmpty())
        , AudioURL(TEXT(""))
        , DisplayDuration(4.0f)
        , NextLineID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString ChoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LeadsToLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredQuestID;

    FNarr_DialogueChoice()
        : ChoiceID(TEXT(""))
        , ChoiceText(FText::GetEmpty())
        , LeadsToLineID(TEXT(""))
        , RequiredQuestID(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DialogueTree
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString TreeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LinkedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RootLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueState State;

    FNarr_DialogueTree()
        : TreeID(TEXT(""))
        , NPCRole(ENarr_NPCRole::Stranger)
        , LinkedQuestID(TEXT(""))
        , TriggerType(ENarr_DialogueTrigger::Proximity)
        , RootLineID(TEXT(""))
        , State(ENarr_DialogueState::Idle)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_LoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LoreID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText LoreText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bDiscovered;

    FNarr_LoreEntry()
        : LoreID(TEXT(""))
        , LocationName(TEXT(""))
        , LoreText(FText::GetEmpty())
        , bDiscovered(false)
    {}
};

// ─── ADialogueManager ────────────────────────────────────────────────────────

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

    // ── Dialogue Trees ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueTree> DialogueTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FNarr_DialogueTree ActiveTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString CurrentLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueState CurrentState;

    // ── Lore System ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    TArray<FNarr_LoreEntry> LoreEntries;

    // ── Audio URLs (ElevenLabs) ─────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString ElderBriefingAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString ScoutFlintAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString CampLeaderDefenseAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Audio")
    FString ElderQuestCompleteAudioURL;

    // ── Functions ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(const FString& TreeID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void MakeChoice(const FString& ChoiceID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueChoice> GetCurrentChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerQuestDialogue(const FString& QuestID, ENarr_DialogueTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    void DiscoverLore(const FString& LoreID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    FNarr_LoreEntry GetLoreEntry(const FString& LoreID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    TArray<FNarr_LoreEntry> GetDiscoveredLore() const;

private:
    void InitialiseDialogueTrees();
    void InitialiseLoreEntries();
    FNarr_DialogueTree* FindTree(const FString& TreeID);
    FNarr_DialogueLine* FindLine(FNarr_DialogueTree& Tree, const FString& LineID);

    float DialogueTimer;
    bool bAutoAdvance;
};
