#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "NarrativeTypes.generated.h"

/**
 * Dialogue emotion states for character expressions and voice modulation
 */
UENUM(BlueprintType)
enum class ENarr_DialogueEmotion : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Wise        UMETA(DisplayName = "Wise"),
    Mystical    UMETA(DisplayName = "Mystical"),
    Urgent      UMETA(DisplayName = "Urgent"),
    Calm        UMETA(DisplayName = "Calm"),
    Reverent    UMETA(DisplayName = "Reverent"),
    Warning     UMETA(DisplayName = "Warning"),
    Teaching    UMETA(DisplayName = "Teaching"),
    Questioning UMETA(DisplayName = "Questioning"),
    Inspiring   UMETA(DisplayName = "Inspiring")
};

/**
 * Character archetypes for the transpersonal journey
 */
UENUM(BlueprintType)
enum class ENarr_CharacterArchetype : uint8
{
    Elder       UMETA(DisplayName = "Tribal Elder"),
    Shaman      UMETA(DisplayName = "Spiritual Shaman"),
    Hunter      UMETA(DisplayName = "Skilled Hunter"),
    Healer      UMETA(DisplayName = "Village Healer"),
    Storyteller UMETA(DisplayName = "Tribal Storyteller"),
    Guardian    UMETA(DisplayName = "Sacred Guardian"),
    Seeker      UMETA(DisplayName = "Truth Seeker"),
    Guide       UMETA(DisplayName = "Spirit Guide")
};

/**
 * Consciousness states that affect dialogue options and narrative flow
 */
UENUM(BlueprintType)
enum class ENarr_ConsciousnessLevel : uint8
{
    Unaware     UMETA(DisplayName = "Unaware"),
    Awakening   UMETA(DisplayName = "Awakening"),
    Aware       UMETA(DisplayName = "Aware"),
    Expanding   UMETA(DisplayName = "Expanding"),
    Transcendent UMETA(DisplayName = "Transcendent")
};

/**
 * Narrative themes that drive quest progression and character development
 */
UENUM(BlueprintType)
enum class ENarr_NarrativeTheme : uint8
{
    SelfDiscovery       UMETA(DisplayName = "Self Discovery"),
    TribalWisdom        UMETA(DisplayName = "Tribal Wisdom"),
    SacredConnection    UMETA(DisplayName = "Sacred Connection"),
    InnerJourney        UMETA(DisplayName = "Inner Journey"),
    AncestralMemory     UMETA(DisplayName = "Ancestral Memory"),
    NaturalHarmony      UMETA(DisplayName = "Natural Harmony"),
    SpiritualAwakening  UMETA(DisplayName = "Spiritual Awakening"),
    CollectiveUnity     UMETA(DisplayName = "Collective Unity")
};

/**
 * Core dialogue line structure for the narrative system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine : public FTableRowBase
{
    GENERATED_BODY()

    FNarr_DialogueLine()
        : SpeakerName(TEXT("Unknown"))
        , DialogueText(TEXT(""))
        , Emotion(ENarr_DialogueEmotion::Neutral)
        , RequiredConsciousnessLevel(ENarr_ConsciousnessLevel::Unaware)
        , AudioAssetPath(TEXT(""))
        , Duration(3.0f)
        , bIsPlayerChoice(false)
    {
    }

    /** Name of the character speaking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    /** The actual dialogue text */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    /** Emotional state for this line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueEmotion Emotion;

    /** Minimum consciousness level required to see this dialogue option */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_ConsciousnessLevel RequiredConsciousnessLevel;

    /** Path to audio asset for voice acting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioAssetPath;

    /** Duration of the dialogue line in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration;

    /** Whether this is a player dialogue choice */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    /** Tags for conditional dialogue logic */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ConditionalTags;
};

/**
 * Character data structure for NPCs with narrative roles
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterData : public FTableRowBase
{
    GENERATED_BODY()

    FNarr_CharacterData()
        : CharacterName(TEXT("Unknown"))
        , Archetype(ENarr_CharacterArchetype::Elder)
        , BackgroundStory(TEXT(""))
        , PrimaryTheme(ENarr_NarrativeTheme::SelfDiscovery)
        , bIsEssentialCharacter(false)
        , VoiceActorName(TEXT(""))
    {
    }

    /** Display name of the character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    /** Character's role archetype */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterArchetype Archetype;

    /** Character's background story and personality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    /** Primary narrative theme this character represents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_NarrativeTheme PrimaryTheme;

    /** Whether this character is essential to the main story */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsEssentialCharacter;

    /** Voice actor or TTS voice ID for this character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString VoiceActorName;

    /** Available dialogue lines for this character */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;
};

/**
 * Narrative event structure for story progression tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    FNarr_StoryEvent()
        : EventID(TEXT(""))
        , EventName(TEXT(""))
        , Description(TEXT(""))
        , Theme(ENarr_NarrativeTheme::SelfDiscovery)
        , bIsCompleted(false)
        , bIsRepeatable(false)
    {
    }

    /** Unique identifier for this story event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    /** Display name of the event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    /** Detailed description of what happens in this event */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    /** Narrative theme this event explores */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_NarrativeTheme Theme;

    /** Whether this event has been completed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    /** Whether this event can be triggered multiple times */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsRepeatable;

    /** Prerequisites for this event to trigger */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    /** Events that become available after completing this one */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedEvents;
};