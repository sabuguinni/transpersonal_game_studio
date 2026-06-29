// NarrativeDialogueData.h
// Agent #15 — Narrative & Dialogue Agent
// Dialogue data structures for NPC_HerdTracker_QuestGiver and waypoint flavour lines
// Cycle: PROD_CYCLE_AUTO_20260629_006

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NarrativeDialogueData.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENarr_DialogueNodeType : uint8
{
    NPC_Line        UMETA(DisplayName = "NPC Line"),
    Player_Choice   UMETA(DisplayName = "Player Choice"),
    Condition_Check UMETA(DisplayName = "Condition Check"),
    End_Conversation UMETA(DisplayName = "End Conversation")
};

UENUM(BlueprintType)
enum class ENarr_HerdTrackerState : uint8
{
    Greeting        UMETA(DisplayName = "Greeting"),
    QuestOffer      UMETA(DisplayName = "Quest Offer"),
    QuestAccepted   UMETA(DisplayName = "Quest Accepted"),
    QuestDeclined   UMETA(DisplayName = "Quest Declined"),
    InProgress      UMETA(DisplayName = "In Progress"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    QuestFailed     UMETA(DisplayName = "Quest Failed")
};

UENUM(BlueprintType)
enum class ENarr_WaypointID : uint8
{
    ValleyEntrance      UMETA(DisplayName = "Valley Entrance"),
    RiverCrossing       UMETA(DisplayName = "River Crossing"),
    HerdRestingGround   UMETA(DisplayName = "Herd Resting Ground"),
    PredatorTerritory   UMETA(DisplayName = "Predator Territory"),
    SouthernPass        UMETA(DisplayName = "Southern Pass")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueNodeType NodeType = ENarr_DialogueNodeType::NPC_Line;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText DialogueText;

    // Audio URL from ElevenLabs/Supabase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioURL;

    // IDs of nodes that follow this one (for branching)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FName> NextNodeIDs;

    // Optional: condition tag that must be true to reach this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName ConditionTag;

    FNarr_DialogueNode()
        : NodeID(NAME_None)
        , NodeType(ENarr_DialogueNodeType::NPC_Line)
        , ConditionTag(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_WaypointFlavourLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Waypoint")
    ENarr_WaypointID WaypointID = ENarr_WaypointID::ValleyEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Waypoint")
    FText FlavourText;

    // Supabase audio URL
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Waypoint")
    FString AudioURL;

    // Speaker — usually the quest giver narrating remotely or an ambient voice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Waypoint")
    FText SpeakerName;

    FNarr_WaypointFlavourLine()
        : WaypointID(ENarr_WaypointID::ValleyEntrance)
    {}
};

USTRUCT(BlueprintType)
struct FNarr_DeathNarrativeBeat
{
    GENERATED_BODY()

    // Cause tag: "Stampede", "Raptor", "TRex", "Starvation", "Drowning"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Death")
    FName CauseTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Death")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Death")
    FString AudioURL;

    // How long to display before respawn prompt (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Death")
    float DisplayDuration = 5.0f;

    FNarr_DeathNarrativeBeat()
        : CauseTag(NAME_None)
        , DisplayDuration(5.0f)
    {}
};

// ─── Main Data Asset ──────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_HerdTrackerDialogueAsset : public UObject
{
    GENERATED_BODY()

public:
    UNarr_HerdTrackerDialogueAsset();

    // Full dialogue tree for NPC_HerdTracker_QuestGiver
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|HerdTracker")
    TArray<FNarr_DialogueNode> DialogueNodes;

    // Waypoint flavour lines (5 waypoints)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Waypoint")
    TArray<FNarr_WaypointFlavourLine> WaypointLines;

    // Death narrative beats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Death")
    TArray<FNarr_DeathNarrativeBeat> DeathBeats;

    // Lore: the Shadow Bull
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FText ShadowBullLoreEntry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Lore")
    FText ShadowBullSpeciesNote;

    // Helper: find node by ID
    UFUNCTION(BlueprintCallable, Category = "Narrative|HerdTracker")
    FNarr_DialogueNode GetNodeByID(FName NodeID) const;

    // Helper: get waypoint flavour line
    UFUNCTION(BlueprintCallable, Category = "Narrative|Waypoint")
    FNarr_WaypointFlavourLine GetWaypointLine(ENarr_WaypointID WaypointID) const;

    // Helper: get death beat by cause
    UFUNCTION(BlueprintCallable, Category = "Narrative|Death")
    FNarr_DeathNarrativeBeat GetDeathBeat(FName CauseTag) const;

    // Populate all data with authored content
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|HerdTracker")
    void PopulateDefaultContent();
};
