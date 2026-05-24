#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NarrativeTypes.generated.h"

UENUM(BlueprintType)
enum class ENarrativeEventType : uint8
{
    Discovery,
    Survival,
    Domestication,
    Threat,
    Memory,
    Revelation,
    Connection
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Wonder,
    Fear,
    Hope,
    Desperation,
    Awe,
    Loneliness,
    Determination,
    Peace
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    FirstEncounter,
    Friendly,
    Hostile,
    Neutral,
    Fearful,
    Curious,
    Protective
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    FNarr_DialogueLine()
        : SpeakerName(TEXT(""))
        , DialogueText(TEXT(""))
        , AudioAssetPath(TEXT(""))
        , EmotionalState(ENarr_EmotionalState::Neutral)
        , ConsciousnessLevel(0)
        , RequiredQuestStage(-1)
        , IsPlayerChoice(false)
        , NextDialogueID(TEXT(""))
    {
    }

    /** Name of the character speaking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    /** The actual dialogue text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    /** Path to audio asset for this line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioAssetPath;

    /** Emotional state of the speaker */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    ENarr_EmotionalState EmotionalState;

    /** Required consciousness level to understand this dialogue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    int32 ConsciousnessLevel;

    /** Quest stage required to see this dialogue (-1 = always available) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredQuestStage;

    /** Is this a player choice option? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    bool IsPlayerChoice;

    /** ID of the next dialogue line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
    FString NextDialogueID;
};

/**
 * Emotional states for dialogue delivery
 */
UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Calm           UMETA(DisplayName = "Calm"),
    Excited        UMETA(DisplayName = "Excited"),
    Angry          UMETA(DisplayName = "Angry"),
    Sad            UMETA(DisplayName = "Sad"),
    Fearful        UMETA(DisplayName = "Fearful"),
    Mystical       UMETA(DisplayName = "Mystical"),
    Wise           UMETA(DisplayName = "Wise"),
    Confused       UMETA(DisplayName = "Confused"),
    Enlightened    UMETA(DisplayName = "Enlightened")
};

/**
 * Story trigger types
 */
UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    LocationEnter   UMETA(DisplayName = "Enter Location"),
    LocationExit    UMETA(DisplayName = "Exit Location"),
    ItemPickup      UMETA(DisplayName = "Item Pickup"),
    NPCInteraction  UMETA(DisplayName = "NPC Interaction"),
    QuestComplete   UMETA(DisplayName = "Quest Complete"),
    ConsciousnessLevel UMETA(DisplayName = "Consciousness Level"),
    TimeOfDay       UMETA(DisplayName = "Time of Day"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    Meditation      UMETA(DisplayName = "Meditation Complete")
};

/**
 * Narrative event data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    FNarr_NarrativeEvent()
        : EventID(TEXT(""))
        , TriggerType(ENarr_TriggerType::LocationEnter)
        , DialogueID(TEXT(""))
        , QuestID(TEXT(""))
        , bIsRepeatable(false)
        , RequiredConsciousnessLevel(0)
    {
    }

    /** Unique identifier for this event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString EventID;

    /** What triggers this narrative event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ENarr_TriggerType TriggerType;

    /** Dialogue to start when triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    /** Quest to start/update when triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    /** Can this event trigger multiple times? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsRepeatable;

    /** Minimum consciousness level required */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    int32 RequiredConsciousnessLevel;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEmotionalTone Tone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceActorDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer RequiredConditions;

    FNarr_DialogueLine()
    {
        Context = ENarr_DialogueContext::Neutral;
        Tone = EEmotionalTone::Wonder;
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarrativeEventType BeatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    EEmotionalTone EmotionalArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggeredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float StoryWeight;

    FNarr_StoryBeat()
    {
        BeatType = ENarrativeEventType::Discovery;
        EmotionalArc = EEmotionalTone::Wonder;
        StoryWeight = 1.0f;
    }
};