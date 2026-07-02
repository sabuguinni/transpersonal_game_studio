// DialogueSystem.h — Narrative & Dialogue Agent #15
// Dialogue manager: NPC trigger volumes, crafting-linked lines, survival journal
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "DialogueSystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_NPCRole : uint8
{
    HunterElder     UMETA(DisplayName = "Hunter Elder"),
    TribeLeader     UMETA(DisplayName = "Tribe Leader"),
    Scout           UMETA(DisplayName = "Scout"),
    Craftsman       UMETA(DisplayName = "Craftsman"),
    Survivor        UMETA(DisplayName = "Survivor")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTrigger : uint8
{
    Proximity       UMETA(DisplayName = "Player Proximity"),
    HasItem         UMETA(DisplayName = "Player Has Item"),
    QuestActive     UMETA(DisplayName = "Quest Active"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    FirstVisit      UMETA(DisplayName = "First Visit Only"),
    Repeatable      UMETA(DisplayName = "Repeatable")
};

UENUM(BlueprintType)
enum class ENarr_JournalCategory : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Dinosaur        UMETA(DisplayName = "Dinosaur Lore"),
    Territory       UMETA(DisplayName = "Territory"),
    Tribe           UMETA(DisplayName = "Tribe")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_DialogueTrigger TriggerCondition = ENarr_DialogueTrigger::Proximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString RequiredItemTag;   // e.g. "StoneAxe", "Campfire" — empty = no requirement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;          // ElevenLabs TTS URL

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bHasBeenPlayed = false;
};

USTRUCT(BlueprintType)
struct FNarr_JournalEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    FString EntryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    FString EntryBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    ENarr_JournalCategory Category = ENarr_JournalCategory::Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    FString UnlockTrigger;     // e.g. "CraftedStoneAxe", "SawTRex"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    bool bIsUnlocked = false;
};

USTRUCT(BlueprintType)
struct FNarr_NPCDialogueSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::HunterElder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 CurrentLineIndex = 0;
};

// ─── NPC Dialogue Actor ────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_NPCDialogueActor : public AActor
{
    GENERATED_BODY()

public:
    ANarr_NPCDialogueActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Trigger volume — player enters to start dialogue
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
    USphereComponent* TriggerSphere;

    // NPC dialogue data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NPCDialogueSet DialogueSet;

    // Crafting-linked lines — shown when player has specific items
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> CraftingLinkedLines;

    // NPC role for identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_NPCRole NPCRole = ENarr_NPCRole::HunterElder;

    // Trigger radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float TriggerRadius = 500.0f;

    // Is player currently in range?
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bPlayerInRange = false;

    // Current active line index
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 ActiveLineIndex = 0;

    // Time remaining on current displayed line
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    float LineDisplayTimer = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNextLine();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCurrentLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasCraftingLine(const FString& ItemTag) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueLine GetCraftingLine(const FString& ItemTag) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ResetDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetTotalLines() const;

private:
    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ─── Dialogue Manager (world singleton) ───────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_DialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_DialogueManager();

    virtual void BeginPlay() override;

    // All journal entries — populated at start, unlocked via triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journal")
    TArray<FNarr_JournalEntry> JournalEntries;

    // All registered NPC dialogue actors
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<ANarr_NPCDialogueActor*> RegisteredNPCs;

    // Unlock a journal entry by trigger tag
    UFUNCTION(BlueprintCallable, Category = "Journal")
    bool UnlockJournalEntry(const FString& TriggerTag);

    // Get all unlocked journal entries
    UFUNCTION(BlueprintCallable, Category = "Journal")
    TArray<FNarr_JournalEntry> GetUnlockedEntries() const;

    // Get entries by category
    UFUNCTION(BlueprintCallable, Category = "Journal")
    TArray<FNarr_JournalEntry> GetEntriesByCategory(ENarr_JournalCategory Category) const;

    // Register an NPC dialogue actor
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterNPC(ANarr_NPCDialogueActor* NPC);

    // Called when player crafts an item — triggers relevant dialogue + journal
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerCraftedItem(const FString& ItemTag);

    // Called when player first sees a dinosaur species
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEncounteredDino(const FString& DinoSpecies);

    // Total journal entries unlocked
    UFUNCTION(BlueprintCallable, Category = "Journal")
    int32 GetUnlockedEntryCount() const;

private:
    void RegisterDefaultJournalEntries();
};
