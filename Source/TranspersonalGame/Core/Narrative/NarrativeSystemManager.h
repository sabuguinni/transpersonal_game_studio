// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Sound/DialogueWave.h"
#include "Sound/DialogueVoice.h"
#include "NarrativeSystemManager.generated.h"

class UDialogueDefinition;
class UStoryBeat;
class ATranspersonalPlayerController;
class UNarrativeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryBeatTriggered, FString, StoryBeatID, FGameplayTagContainer, StoryTags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueStarted, FString, DialogueID, class UDialogueVoice*, Speaker, FText, OpeningLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, FString, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMemoryUnlocked, FString, MemoryID, FText, MemoryDescription);

/**
 * Story Beat Type - different types of narrative moments
 */
UENUM(BlueprintType)
enum class EStoryBeatType : uint8
{
    Discovery           UMETA(DisplayName = "Discovery"),        // Finding something new
    Revelation          UMETA(DisplayName = "Revelation"),      // Learning important info
    Conflict            UMETA(DisplayName = "Conflict"),        // Dangerous encounter
    Resolution          UMETA(DisplayName = "Resolution"),      // Solving a problem
    Transition          UMETA(DisplayName = "Transition"),      // Moving between areas
    Memory              UMETA(DisplayName = "Memory"),          // Flashback/remembering
    Choice              UMETA(DisplayName = "Choice"),          // Player decision point
    Observation         UMETA(DisplayName = "Observation"),     // Passive story moment
    Environmental       UMETA(DisplayName = "Environmental"),   // World tells story
    Character           UMETA(DisplayName = "Character"),       // Character development
    
    MAX                 UMETA(Hidden)
};

/**
 * Dialogue Context Type - who is speaking and in what situation
 */
UENUM(BlueprintType)
enum class EDialogueContext : uint8
{
    InternalMonologue   UMETA(DisplayName = "Internal Monologue"), // Player's thoughts
    Discovery           UMETA(DisplayName = "Discovery"),          // Finding something
    Observation         UMETA(DisplayName = "Observation"),        // Watching dinosaurs
    Memory              UMETA(DisplayName = "Memory"),             // Remembering past
    Fear                UMETA(DisplayName = "Fear"),               // Scared/threatened
    Wonder              UMETA(DisplayName = "Wonder"),             // Amazed by world
    Determination       UMETA(DisplayName = "Determination"),      // Focused on survival
    Loneliness          UMETA(DisplayName = "Loneliness"),         // Missing home
    Scientific          UMETA(DisplayName = "Scientific"),         // Analyzing as scientist
    Survival            UMETA(DisplayName = "Survival"),           // Basic needs focus
    
    MAX                 UMETA(Hidden)
};

/**
 * Memory Type - different categories of memories the player can unlock
 */
UENUM(BlueprintType)
enum class EMemoryType : uint8
{
    PersonalLife        UMETA(DisplayName = "Personal Life"),     // Family, friends
    ProfessionalLife    UMETA(DisplayName = "Professional Life"), // Career, research
    TheDiscovery        UMETA(DisplayName = "The Discovery"),     // Finding the gem
    ScientificKnowledge UMETA(DisplayName = "Scientific Knowledge"), // Paleontology facts
    ModernWorld         UMETA(DisplayName = "Modern World"),      // What he left behind
    Motivation          UMETA(DisplayName = "Motivation"),        // Why he wants to return
    Skills              UMETA(DisplayName = "Skills"),            // Abilities he remembers
    Fears               UMETA(DisplayName = "Fears"),             // What he's afraid of
    
    MAX                 UMETA(Hidden)
};

/**
 * Story Beat Definition - a narrative moment in the game
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryBeat
{
    GENERATED_BODY()

    // Basic story beat info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString StoryBeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText StoryBeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText StoryBeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    EStoryBeatType BeatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FGameplayTagContainer StoryTags;

    // Trigger conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    FGameplayTagContainer RequiredPlayerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    FGameplayTagContainer RequiredWorldState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    TArray<FString> RequiredCompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
    float CooldownTime;

    // Narrative content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    TArray<FString> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    TArray<class UDialogueWave*> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    FText SubtitleOverride;

    // Effects and consequences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FGameplayTagContainer TagsToAdd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FGameplayTagContainer TagsToRemove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<FString> QuestsToActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<FString> MemoriesToUnlock;

    // State tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bHasTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float LastTriggerTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    int32 TriggerCount;

    FStoryBeat()
    {
        StoryBeatID = TEXT("");
        StoryBeatTitle = FText::GetEmpty();
        StoryBeatDescription = FText::GetEmpty();
        BeatType = EStoryBeatType::Observation;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bCanRepeat = false;
        CooldownTime = 0.0f;
        bHasTriggered = false;
        LastTriggerTime = 0.0f;
        TriggerCount = 0;
    }
};

/**
 * Memory Definition - represents a piece of the player's past
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString MemoryID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FText MemoryTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FText MemoryDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FText MemoryNarration; // Longer text for memory sequences

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    EMemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FGameplayTagContainer MemoryTags;

    // Unlock conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
    TArray<FString> RequiredStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
    FVector UnlockLocation; // Specific location that triggers memory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
    float UnlockRadius;

    // Memory content
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    class UDialogueWave* MemoryVoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
    TArray<FText> MemoryFragments; // Multiple pieces of the memory

    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FGameplayTagContainer TagsGranted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<FString> QuestsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    int32 ExperienceGranted;

    // State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float UnlockTime;

    FMemoryDefinition()
    {
        MemoryID = TEXT("");
        MemoryTitle = FText::GetEmpty();
        MemoryDescription = FText::GetEmpty();
        MemoryNarration = FText::GetEmpty();
        MemoryType = EMemoryType::PersonalLife;
        UnlockLocation = FVector::ZeroVector;
        UnlockRadius = 100.0f;
        ExperienceGranted = 0;
        bIsUnlocked = false;
        UnlockTime = 0.0f;
    }
};

/**
 * Dialogue Line - individual line of dialogue with context
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueWave* VoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueVoice* Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DelayBeforeLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LineDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsThought; // Internal monologue vs spoken

    FDialogueLine()
    {
        DialogueText = FText::GetEmpty();
        Context = EDialogueContext::InternalMonologue;
        VoiceLine = nullptr;
        Speaker = nullptr;
        DelayBeforeLine = 0.0f;
        LineDuration = 3.0f;
        bIsThought = true;
    }
};

/**
 * Narrative System Manager - manages all narrative content and story progression
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnStoryBeatTriggered OnStoryBeatTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnMemoryUnlocked OnMemoryUnlocked;

    // Story Beat Management
    UFUNCTION(BlueprintCallable, Category = "Story Beats")
    void RegisterStoryBeat(const FStoryBeat& StoryBeat);

    UFUNCTION(BlueprintCallable, Category = "Story Beats")
    bool TriggerStoryBeat(const FString& StoryBeatID, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Story Beats")
    void CheckLocationBasedStoryBeats(const FVector& PlayerLocation, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Story Beats")
    TArray<FStoryBeat> GetActiveStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Story Beats")
    FStoryBeat GetStoryBeat(const FString& StoryBeatID) const;

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RegisterMemory(const FMemoryDefinition& Memory);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool UnlockMemory(const FString& MemoryID, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void CheckMemoryUnlocks(const FVector& PlayerLocation, const FGameplayTagContainer& PlayerTags, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FMemoryDefinition> GetUnlockedMemories() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FMemoryDefinition GetMemory(const FString& MemoryID) const;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FDialogueLine& DialogueLine, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayInternalMonologue(const FText& MonologueText, EDialogueContext Context, APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialoguePlaying() const;

    // Narrative State
    UFUNCTION(BlueprintCallable, Category = "Narrative State")
    void SetNarrativeTag(const FGameplayTag& Tag, bool bAdd = true);

    UFUNCTION(BlueprintCallable, Category = "Narrative State")
    bool HasNarrativeTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative State")
    FGameplayTagContainer GetNarrativeTags() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative State")
    void ClearNarrativeTags();

    // Environmental Storytelling
    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void TriggerEnvironmentalNarrative(const FVector& Location, const FString& NarrativeID);

    UFUNCTION(BlueprintCallable, Category = "Environmental")
    void RegisterEnvironmentalStoryPoint(const FVector& Location, const FString& StoryText, float TriggerRadius = 200.0f);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Save System")
    void SaveNarrativeState();

    UFUNCTION(BlueprintCallable, Category = "Save System")
    void LoadNarrativeState();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugPrintNarrativeState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ForceUnlockAllMemories();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ResetNarrativeProgress();

protected:
    // Story beat storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Data")
    TMap<FString, FStoryBeat> StoryBeats;

    // Memory storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Data")
    TMap<FString, FMemoryDefinition> Memories;

    // Narrative state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative State")
    FGameplayTagContainer CurrentNarrativeTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative State")
    TArray<FString> CompletedStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative State")
    TArray<FString> UnlockedMemories;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    bool bIsDialoguePlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue State")
    float DialogueStartTime;

    // Environmental story points
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    TMap<FVector, FString> EnvironmentalStoryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    TMap<FVector, float> EnvironmentalTriggerRadii;

    // Dialogue voice references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Setup")
    class UDialogueVoice* PlayerVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Setup")
    class UDialogueVoice* NarratorVoice;

    // Internal functions
    bool CheckStoryBeatConditions(const FStoryBeat& StoryBeat, APlayerController* PlayerController) const;
    bool CheckMemoryUnlockConditions(const FMemoryDefinition& Memory, const FGameplayTagContainer& PlayerTags) const;
    void ApplyStoryBeatEffects(const FStoryBeat& StoryBeat, APlayerController* PlayerController);
    void ApplyMemoryEffects(const FMemoryDefinition& Memory, APlayerController* PlayerController);

private:
    // Timer handles for dialogue management
    FTimerHandle DialogueTimerHandle;
    FTimerHandle MemoryCheckTimerHandle;
    FTimerHandle StoryBeatCheckTimerHandle;

    // Internal state
    float LastLocationCheckTime;
    FVector LastPlayerLocation;
    
    // Constants
    static constexpr float LOCATION_CHECK_INTERVAL = 1.0f;
    static constexpr float MEMORY_CHECK_INTERVAL = 2.0f;
    static constexpr float STORY_BEAT_CHECK_INTERVAL = 0.5f;
};