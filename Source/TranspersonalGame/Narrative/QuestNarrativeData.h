#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestNarrativeData.generated.h"

/**
 * Narrative data for the 3 core quests in MinPlayableMap.
 * Each quest has: briefing lines, mid-quest lines, completion lines, failure lines.
 * All audio URLs reference ElevenLabs TTS assets uploaded to Supabase.
 * Agent #15 — Narrative & Dialogue Agent
 */

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_QuestPhase : uint8
{
    Briefing    UMETA(DisplayName = "Briefing"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class ENarr_QuestID : uint8
{
    Hunt_RaptorPack   UMETA(DisplayName = "Hunt: Raptor Pack"),
    Gather_Flint      UMETA(DisplayName = "Gather: Flint"),
    Defend_Camp       UMETA(DisplayName = "Defend: Camp")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    /** Speaker label shown in HUD (e.g. "Elder Kara") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString SpeakerName;

    /** The dialogue text displayed on screen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    /** Supabase URL for the TTS audio file */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString AudioURL;

    /** How long to display this line (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DisplayDuration = 5.0f;

    FNarr_DialogueLine() {}

    FNarr_DialogueLine(const FString& InSpeaker, const FString& InText, const FString& InURL, float InDuration = 5.0f)
        : SpeakerName(InSpeaker), DialogueText(InText), AudioURL(InURL), DisplayDuration(InDuration)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_QuestDialogue
{
    GENERATED_BODY()

    /** Which quest this dialogue belongs to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_QuestID QuestID = ENarr_QuestID::Hunt_RaptorPack;

    /** Lines played when quest is first triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> BriefingLines;

    /** Lines played during quest (e.g. on objective progress) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> ProgressLines;

    /** Lines played on quest completion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> CompletionLines;

    /** Lines played on quest failure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_DialogueLine> FailureLines;
};

// ─── Data Object ─────────────────────────────────────────────────────────────

/**
 * UNarr_QuestNarrativeData
 * DataAsset-style UObject holding all dialogue for the 3 core quests.
 * Populated at runtime by UNarr_QuestNarrativeLibrary::BuildAllQuestDialogue().
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_QuestNarrativeData : public UObject
{
    GENERATED_BODY()

public:
    UNarr_QuestNarrativeData();

    /** All quest dialogues indexed by ENarr_QuestID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_QuestDialogue> AllQuestDialogues;

    /** Get dialogue for a specific quest */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_QuestDialogue GetQuestDialogue(ENarr_QuestID QuestID) const;

    /** Get briefing lines for a quest */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetBriefingLines(ENarr_QuestID QuestID) const;

    /** Get completion lines for a quest */
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_DialogueLine> GetCompletionLines(ENarr_QuestID QuestID) const;
};
