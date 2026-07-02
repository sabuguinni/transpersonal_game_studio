#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StampedeNarrativeData.generated.h"

// ============================================================
// ENarr_StampedePhase — mirrors EQuest_SurvivorState from QuestManager
// Used by dialogue system to select contextual lines
// ============================================================
UENUM(BlueprintType)
enum class ENarr_StampedePhase : uint8
{
	PreQuest        UMETA(DisplayName = "Pre-Quest Briefing"),
	WatchingHerd    UMETA(DisplayName = "Watching the Herd"),
	EscapeWindow    UMETA(DisplayName = "Escape Window Open"),
	StampedeActive  UMETA(DisplayName = "Stampede Active"),
	Completed       UMETA(DisplayName = "Quest Completed"),
	Failed          UMETA(DisplayName = "Quest Failed")
};

// ============================================================
// FNarr_DialogueLine — single spoken line with metadata
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_StampedeDialogueLine
{
	GENERATED_BODY()

	/** Phase during which this line is valid */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	ENarr_StampedePhase Phase = ENarr_StampedePhase::PreQuest;

	/** Speaker identifier — matches NPC tag in level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	FName SpeakerID = NAME_None;

	/** Display name shown in subtitle bar */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	FText SpeakerDisplayName;

	/** Full dialogue text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	FText LineText;

	/** ElevenLabs TTS audio URL — streamed at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	FString AudioURL;

	/** Subtitle display duration in seconds (0 = auto from text length) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	float DisplayDuration = 0.0f;

	/** Priority — higher priority lines interrupt lower ones */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	int32 Priority = 0;
};

// ============================================================
// FNarr_StampedeLore — lore context for the quest
// ============================================================
USTRUCT(BlueprintType)
struct FNarr_StampedeLore
{
	GENERATED_BODY()

	/** Name of the valley where the quest takes place */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText ValleyName;

	/** Species of the herd (Parasaurolophus — large hadrosaur) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText HerdSpecies;

	/** Estimated herd size for environmental storytelling */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	int32 HerdSize = 0;

	/** Predator that triggers the stampede */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText PredatorSpecies;

	/** Why the predator is in this area (migration reason) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText PredatorMigrationReason;

	/** Season — affects herd behaviour and player survival context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText Season;

	/** Journal entry written by the player character after surviving */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FText PlayerJournalEntry;
};

// ============================================================
// UNarr_StampedeNarrativeData — UObject data asset
// Holds all dialogue lines and lore for the Survive the Stampede quest
// Loaded by DinosaurBehaviorDialogue system at runtime
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StampedeNarrativeData : public UObject
{
	GENERATED_BODY()

public:
	UNarr_StampedeNarrativeData();

	// ---- Dialogue Lines ----

	/** All dialogue lines for this quest, indexed by phase */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Stampede")
	TArray<FNarr_StampedeDialogueLine> DialogueLines;

	/** Lore context for environmental storytelling */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative|Lore")
	FNarr_StampedeLore LoreContext;

	// ---- Blueprint Accessors ----

	/** Get all lines valid for a given quest phase */
	UFUNCTION(BlueprintCallable, Category = "Narrative|Stampede")
	TArray<FNarr_StampedeDialogueLine> GetLinesForPhase(ENarr_StampedePhase Phase) const;

	/** Get the highest-priority line for a given phase */
	UFUNCTION(BlueprintCallable, Category = "Narrative|Stampede")
	bool GetPriorityLineForPhase(ENarr_StampedePhase Phase, FNarr_StampedeDialogueLine& OutLine) const;

	/** Get the player journal entry text */
	UFUNCTION(BlueprintPure, Category = "Narrative|Lore")
	FText GetJournalEntry() const { return LoreContext.PlayerJournalEntry; }

	/** Get lore context struct */
	UFUNCTION(BlueprintPure, Category = "Narrative|Lore")
	FNarr_StampedeLore GetLoreContext() const { return LoreContext; }

#if WITH_EDITOR
	/** Populate default dialogue lines and lore — call in editor to seed the data asset */
	UFUNCTION(CallInEditor, Category = "Narrative|Debug")
	void PopulateDefaultData();
#endif
};
