#pragma once

#include "CoreMinimal.h"
#include "DialogueTypes.generated.h"

// ============================================================
// Narr_ prefixed types — Agent #15 Narrative & Dialogue
// All types unique-prefixed to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class ENarr_DialogueSpeaker : uint8
{
    None            UMETA(DisplayName = "None"),
    HuntLeader      UMETA(DisplayName = "Hunt Leader"),
    SurvivalScout   UMETA(DisplayName = "Survival Scout"),
    NightWatcher    UMETA(DisplayName = "Night Watcher"),
    TribeElder      UMETA(DisplayName = "Tribe Elder"),
    ElderHunter     UMETA(DisplayName = "Elder Hunter"),
    ScoutWarrior    UMETA(DisplayName = "Scout Warrior"),
    Player          UMETA(DisplayName = "Player")
};

UENUM(BlueprintType)
enum class ENarr_DialogueTriggerType : uint8
{
    ProximityEnter  UMETA(DisplayName = "Proximity Enter"),
    QuestStart      UMETA(DisplayName = "Quest Start"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    DinosaurSighted UMETA(DisplayName = "Dinosaur Sighted"),
    PlayerDanger    UMETA(DisplayName = "Player Danger"),
    ResourceFound   UMETA(DisplayName = "Resource Found")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Playing     UMETA(DisplayName = "Playing"),
    Waiting     UMETA(DisplayName = "Waiting for Input"),
    Completed   UMETA(DisplayName = "Completed"),
    Interrupted UMETA(DisplayName = "Interrupted")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FText LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FString AudioAssetPath;

    /** Duration in seconds — 0 = auto from audio length */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    float DisplayDuration = 0.0f;

    /** If true, pauses game time while this line plays */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPausesGameplay = false;
};

USTRUCT(BlueprintType)
struct FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    FName SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    TArray<FNarr_DialogueLine> Lines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    ENarr_DialogueTriggerType TriggerType = ENarr_DialogueTriggerType::ProximityEnter;

    /** Once played, won't replay unless reset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Dialogue")
    bool bPlayOnce = true;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|Dialogue")
    bool bHasPlayed = false;
};

USTRUCT(BlueprintType)
struct FNarr_NPCVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    ENarr_DialogueSpeaker Speaker = ENarr_DialogueSpeaker::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString ElevenLabsVoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    float VolumeMultiplier = 1.0f;
};
