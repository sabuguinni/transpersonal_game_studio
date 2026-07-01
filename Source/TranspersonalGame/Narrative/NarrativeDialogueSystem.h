#pragma once

// NarrativeDialogueSystem.h
// Narrative & Dialogue Agent #15 — Transpersonal Game Studio
// Full header for dialogue component and narrative manager.
// Prefix: Narr_ (all types unique per UE5 compilation rules)

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeDialogueSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    Proximity           UMETA(DisplayName = "Proximity"),
    QuestStart          UMETA(DisplayName = "Quest Start"),
    QuestComplete       UMETA(DisplayName = "Quest Complete"),
    CombatAlert         UMETA(DisplayName = "Combat Alert"),
    ResourceFound       UMETA(DisplayName = "Resource Found"),
    CraftingComplete    UMETA(DisplayName = "Crafting Complete"),
    PlayerDeath         UMETA(DisplayName = "Player Death"),
    FirstEntry          UMETA(DisplayName = "First Entry to Zone")
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

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SpeakerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText DialogueText;

    // URL to ElevenLabs-generated audio (Supabase Storage)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    // Path to USoundBase asset if imported into Content Browser
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TSoftObjectPtr<USoundBase> AudioAsset;

    // How long to display this line (0 = wait for player input)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDurationSeconds = 0.0f;

    // Optional camera focus tag on speaker actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName CameraFocusTag;
};

USTRUCT(BlueprintType)
struct FNarr_Conversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    FName ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    TArray<FNarr_DialogueLine> Lines;

    // If true, this conversation only plays once per save
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    bool bOneShot = true;

    // If true, lines auto-advance using DisplayDurationSeconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    bool bAutoAdvance = false;

    // Narrative flag set when this conversation completes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    FName CompletionFlag;

    // Quest objective ID to mark complete on conversation end
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Conversation")
    FName QuestObjectiveID;
};

USTRUCT(BlueprintType)
struct FNarr_LoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FName EntryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FText BodyText;

    // Condition flag that must be set to unlock this entry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FName UnlockCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    bool bIsUnlocked = false;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueStarted, FName, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnDialogueEnded, FName, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnLineDisplayed, const FNarr_DialogueLine&, Line);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNarr_OnNarrativeFlagChanged, FName, FlagName, bool, bValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNarr_OnNarrativeEventTriggered, FName, EventID);

// ============================================================
// UNarr_DialogueComponent — Attach to NPC actors
// ============================================================

UCLASS(ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UNarr_DialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ---- Conversation Library ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_Conversation> ConversationLibrary;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    bool bIsDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue", meta = (AllowPrivateAccess = "true"))
    FNarr_Conversation ActiveConversation;

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    ENarr_SpeakerRole DefaultSpeakerRole;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnLineDisplayed OnLineDisplayed;

    // ---- Methods ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(const FName& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    bool CanTriggerDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void AddConversation(const FNarr_Conversation& Conversation);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FNarr_DialogueLine GetCurrentLine() const;

private:
    void DisplayCurrentLine();

    FTimerHandle AutoAdvanceTimerHandle;
};

// ============================================================
// UNarr_QuestNarrativeManager — GameInstance Subsystem
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_QuestNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Lore Database ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    void RegisterLoreEntry(const FNarr_LoreEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Lore")
    bool GetLoreEntry(const FName& EntryID, FNarr_LoreEntry& OutEntry) const;

    // ---- Narrative Flags ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Flags")
    void SetNarrativeFlag(const FName& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Flags")
    bool GetNarrativeFlag(const FName& FlagName) const;

    // ---- Events ----
    UFUNCTION(BlueprintCallable, Category = "Narrative|Events")
    void TriggerNarrativeEvent(const FName& EventID);

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnNarrativeFlagChanged OnNarrativeFlagChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FNarr_OnNarrativeEventTriggered OnNarrativeEventTriggered;

private:
    UPROPERTY()
    TMap<FName, FNarr_LoreEntry> LoreDatabase;

    UPROPERTY()
    TMap<FName, bool> ActiveNarrativeFlags;

    void RegisterDefaultLoreEntries();
};
