#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.h"
#include "NarrativeBible.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNarrativeBible, Log, All);

// Enums for narrative context
UENUM(BlueprintType)
enum class ENarrativeEmotion : uint8
{
    Wonder      UMETA(DisplayName = "Wonder"),
    Fear        UMETA(DisplayName = "Fear"),
    Melancholy  UMETA(DisplayName = "Melancholy"),
    Relief      UMETA(DisplayName = "Relief"),
    Curiosity   UMETA(DisplayName = "Curiosity"),
    Loneliness  UMETA(DisplayName = "Loneliness"),
    Awe         UMETA(DisplayName = "Awe"),
    Determination UMETA(DisplayName = "Determination")
};

UENUM(BlueprintType)
enum class ENarrativeTimeContext : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night"),
    Storm       UMETA(DisplayName = "Storm"),
    Calm        UMETA(DisplayName = "Calm")
};

UENUM(BlueprintType)
enum class ENarrativeRole : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    Antagonist      UMETA(DisplayName = "Antagonist"),
    Ally            UMETA(DisplayName = "Ally"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Environment     UMETA(DisplayName = "Environment"),
    Narrator        UMETA(DisplayName = "Narrator")
};

UENUM(BlueprintType)
enum class ENarrativeTriggerType : uint8
{
    TimeBasedEvent      UMETA(DisplayName = "Time Based Event"),
    LocationDiscovery   UMETA(DisplayName = "Location Discovery"),
    EmotionalThreshold  UMETA(DisplayName = "Emotional Threshold"),
    CharacterInteraction UMETA(DisplayName = "Character Interaction"),
    QuestCompletion     UMETA(DisplayName = "Quest Completion"),
    EnvironmentalEvent  UMETA(DisplayName = "Environmental Event")
};

// Structs for narrative data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeCharacter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarrativeRole NarrativeRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TMap<FString, FString> CharacterTraits;

    FNarrativeCharacter()
    {
        Name = TEXT("");
        Role = TEXT("");
        Description = TEXT("");
        NarrativeRole = ENarrativeRole::Neutral;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeAct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
    FString ActName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
    float StartProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
    float EndProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Act")
    TArray<FString> KeyEvents;

    FNarrativeAct()
    {
        ActName = TEXT("");
        Description = TEXT("");
        StartProgress = 0.0f;
        EndProgress = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeProgressionPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    float ProgressValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    ENarrativeEmotion AssociatedEmotion;

    FNarrativeProgressionPoint()
    {
        ProgressValue = 0.0f;
        EventName = TEXT("");
        Description = TEXT("");
        AssociatedEmotion = ENarrativeEmotion::Wonder;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString StartingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString EndingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    FString CentralConflict;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Arc")
    TArray<FNarrativeProgressionPoint> ProgressionPoints;

    FCharacterArc()
    {
        CharacterName = TEXT("");
        StartingState = TEXT("");
        EndingState = TEXT("");
        CentralConflict = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCoreNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Narrative")
    FNarrativeCharacter Protagonist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Narrative")
    FString CentralConflict;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Narrative")
    FString ThematicCore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Narrative")
    TArray<FNarrativeAct> ActStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Narrative")
    TMap<FString, FString> ThematicElements;

    FCoreNarrative()
    {
        CentralConflict = TEXT("");
        ThematicCore = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLoreEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString ShortDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString DetailedDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    TArray<FString> RelatedEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    bool bDiscovered;

    FLoreEntry()
    {
        Title = TEXT("");
        ShortDescription = TEXT("");
        DetailedDescription = TEXT("");
        bDiscovered = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString TriggerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ENarrativeTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float ProgressionValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bTriggered;

    // Trigger condition parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Conditions")
    float TimeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Conditions")
    FString LocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Conditions")
    float EmotionalThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger Conditions")
    FString CharacterName;

    FNarrativeTrigger()
    {
        TriggerName = TEXT("");
        TriggerType = ENarrativeTriggerType::TimeBasedEvent;
        NarrativeText = TEXT("");
        ProgressionValue = 0.0f;
        bTriggered = false;
        TimeThreshold = 0.0f;
        LocationName = TEXT("");
        EmotionalThreshold = 0.5f;
        CharacterName = TEXT("");
    }
};

// Delegates for narrative events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEmotionalStateChanged, ENarrativeEmotion, FromEmotion, ENarrativeEmotion, ToEmotion, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeTriggerExecuted, FString, TriggerName, FString, NarrativeText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActChanged, FString, NewAct, FString, ActDescription);

/**
 * Comprehensive Narrative Bible system that manages the core story structure,
 * character arcs, emotional states, and contextual narrative generation.
 * This is the authoritative source for all narrative content in the game.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeBible : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeBible();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    // Core narrative data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Bible")
    FCoreNarrative CoreNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Bible")
    TMap<FString, FCharacterArc> CharacterArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Bible")
    TMap<FString, FLoreEntry> WorldLore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Bible")
    TArray<FNarrativeTrigger> NarrativeTriggers;

    // Current narrative state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ENarrativeEmotion CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ENarrativeTimeContext CurrentTimeContext;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float EmotionalIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float StoryProgression;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FString CurrentAct;

    // Contextual narratives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Contextual Narratives")
    TMap<FString, FString> ContextualNarratives;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNarrativeTriggerExecuted OnNarrativeTriggerExecuted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnActChanged OnActChanged;

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    FString GetContextualNarrative(const FString& Context) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    void UpdateStoryProgression(float ProgressionValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    ENarrativeEmotion GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    float GetStoryProgression() const { return StoryProgression; }

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    FString GetCurrentAct() const { return CurrentAct; }

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    const FLoreEntry* GetLoreEntry(const FString& LoreKey) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Bible")
    void DiscoverLoreEntry(const FString& LoreKey);

private:
    // Initialization functions
    void InitializeCoreNarrative();
    void InitializeCharacterArcs();
    void InitializeWorldLore();

    // Update functions
    void UpdateNarrativeContext(float DeltaTime);
    void ProcessEmotionalTransitions(float DeltaTime);
    void CheckNarrativeTriggers();

    // Emotional state management
    ENarrativeEmotion DetermineEmotionalState() const;
    void TriggerEmotionalTransition(ENarrativeEmotion FromEmotion, ENarrativeEmotion ToEmotion);

    // Narrative trigger system
    bool EvaluateTriggerCondition(const FNarrativeTrigger& Trigger) const;
    void ExecuteNarrativeTrigger(const FNarrativeTrigger& Trigger);

    // Context-specific responses
    void TriggerFearResponse();
    void TriggerWonderResponse();
    void TriggerMelancholyResponse();
    void TriggerReliefResponse();

    // Helper functions
    FString GetDefaultNarrativeForContext(const FString& Context) const;
    void UpdateSurvivalContext();
    void UpdateDiscoveryContext();
    bool CheckLocationDiscovered(const FString& LocationName) const;
    bool CheckCharacterInteraction(const FString& CharacterName) const;
};