#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NarrativeTypes.generated.h"

/**
 * Dialogue line data structure
 */
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

/**
 * Character archetype for dialogue personality
 */
UENUM(BlueprintType)
enum class ENarr_CharacterArchetype : uint8
{
    Elder           UMETA(DisplayName = "Wise Elder"),
    Shaman          UMETA(DisplayName = "Mystical Shaman"),
    Hunter          UMETA(DisplayName = "Skilled Hunter"),
    Gatherer        UMETA(DisplayName = "Knowledge Gatherer"),
    Child           UMETA(DisplayName = "Innocent Child"),
    Warrior         UMETA(DisplayName = "Brave Warrior"),
    Healer          UMETA(DisplayName = "Spiritual Healer"),
    Storyteller     UMETA(DisplayName = "Tribal Storyteller"),
    Outcast         UMETA(DisplayName = "Mysterious Outcast"),
    Spirit          UMETA(DisplayName = "Ancestral Spirit")
};

/**
 * Consciousness themes for narrative content
 */
UENUM(BlueprintType)
enum class ENarr_ConsciousnessTheme : uint8
{
    Awakening       UMETA(DisplayName = "Spiritual Awakening"),
    Unity           UMETA(DisplayName = "Universal Unity"),
    Transformation  UMETA(DisplayName = "Personal Transformation"),
    Wisdom          UMETA(DisplayName = "Ancient Wisdom"),
    Balance         UMETA(DisplayName = "Natural Balance"),
    Transcendence   UMETA(DisplayName = "Transcendence"),
    Connection      UMETA(DisplayName = "Soul Connection"),
    Purpose         UMETA(DisplayName = "Life Purpose"),
    Healing         UMETA(DisplayName = "Spiritual Healing"),
    Evolution       UMETA(DisplayName = "Consciousness Evolution")
};