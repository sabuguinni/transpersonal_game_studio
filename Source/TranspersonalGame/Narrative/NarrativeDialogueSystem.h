// NarrativeDialogueSystem.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue trigger, playback, and quest arc binding for the "First Tools" tutorial.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NarrativeDialogueSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — must be at global scope (UE5 compilation rule)
// Prefix: ENarr_ to avoid collisions with other agents
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity       UMETA(DisplayName = "Proximity"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    CombatAlert     UMETA(DisplayName = "Combat Alert"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    DayBreak        UMETA(DisplayName = "Day Break"),
    Scripted        UMETA(DisplayName = "Scripted")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerRole : uint8
{
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Narrator        UMETA(DisplayName = "Narrator")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — global scope, Narr_ prefix
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue", meta = (MultiLine = "true"))
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 4.0f;
};

USTRUCT(BlueprintType)
struct FNarr_Conversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    FName ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Scripted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    bool bIsRepeatable = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// DELEGATES
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnConversationEvent, FName, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueLineEvent, FNarr_DialogueLine, Line);

// ─────────────────────────────────────────────────────────────────────────────
// UCLASS — UNarrDialogueSystem
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), DisplayName = "Narr Dialogue System")
class TRANSPERSONALGAME_API UNarrDialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrDialogueSystem();

protected:
    virtual void BeginPlay() override;

    // ── Internal state ──────────────────────────────────────────────────────
    UPROPERTY()
    TMap<FName, FNarr_Conversation> Conversations;

    UPROPERTY()
    TSet<FName> PlayedConversations;

    UPROPERTY()
    FNarr_Conversation ActiveConversation;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex;

    // ── Internal helpers ────────────────────────────────────────────────────
    void PlayCurrentLine();
    void AdvanceDialogue();
    void EndConversation();
    void InitializeDefaultConversations();

public:
    // ── Configuration ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bAutoAdvance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float AutoAdvanceDelay = 0.5f;

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnConversationEvent OnConversationStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnConversationEvent OnConversationEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineEvent OnDialogueLineStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueLineEvent OnDialogueLineEnded;

    // ── Public API ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool StartConversation(FName ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void SkipCurrentLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void TriggerConversationByType(ENarr_DialogueTriggerType TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AddConversation(const FNarr_Conversation& NewConversation);

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsConversationPlayed(FName ConversationID) const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    float GetConversationProgress() const;

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialoguePlaying() const { return bIsDialoguePlaying; }

    UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
    int32 GetCurrentLineIndex() const { return CurrentLineIndex; }
};
