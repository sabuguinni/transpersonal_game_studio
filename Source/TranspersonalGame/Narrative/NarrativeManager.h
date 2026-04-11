#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class EStoryState : uint8
{
    Prologue        UMETA(DisplayName = "Prologue"),
    TribalAwakening UMETA(DisplayName = "Tribal Awakening"),
    FirstVision     UMETA(DisplayName = "First Vision"),
    ShamanTraining  UMETA(DisplayName = "Shaman Training"),
    SpiritQuest     UMETA(DisplayName = "Spirit Quest"),
    Transformation  UMETA(DisplayName = "Transformation"),
    Ascension       UMETA(DisplayName = "Ascension"),
    Epilogue        UMETA(DisplayName = "Epilogue")
};

UENUM(BlueprintType)
enum class EDialogueType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Lore            UMETA(DisplayName = "Lore"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Ambient         UMETA(DisplayName = "Ambient"),
    Meditation      UMETA(DisplayName = "Meditation")
};

USTRUCT(BlueprintType)
struct FDialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> RequiredStoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> SetStoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* VoiceAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimSequence* SpeakerAnimation;

    FDialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        DialogueType = EDialogueType::Ambient;
        VoiceAudio = nullptr;
        SpeakerAnimation = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FStoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    EStoryState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> ActiveStoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, int32> CharacterRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TArray<FString> UnlockedAbilities;

    FStoryProgress()
    {
        CurrentState = EStoryState::Prologue;
        ConsciousnessLevel = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryStateChanged, EStoryState, OldState, EStoryState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FDialogueEntry&, DialogueEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

/**
 * Central manager for all narrative elements in the Transpersonal Game
 * Handles story progression, dialogue system, character relationships, and consciousness evolution
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Story State Management
    UFUNCTION(BlueprintCallable, Category = "Narrative|Story")
    void SetStoryState(EStoryState NewState);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Story")
    EStoryState GetCurrentStoryState() const { return StoryProgress.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Story")
    void AddStoryFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Story")
    void RemoveStoryFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Story")
    bool HasStoryFlag(const FString& FlagName) const;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Narrative|Quests")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Quests")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
    FDialogueEntry GetDialogueEntry(const FString& DialogueID);

    // Character Relationships
    UFUNCTION(BlueprintCallable, Category = "Narrative|Characters")
    void ModifyCharacterRelationship(const FString& CharacterName, int32 RelationshipChange);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Characters")
    int32 GetCharacterRelationship(const FString& CharacterName) const;

    // Consciousness System Integration
    UFUNCTION(BlueprintCallable, Category = "Narrative|Consciousness")
    void UpdateConsciousnessLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Narrative|Consciousness")
    float GetConsciousnessLevel() const { return StoryProgress.ConsciousnessLevel; }

    UFUNCTION(BlueprintCallable, Category = "Narrative|Consciousness")
    void UnlockAbility(const FString& AbilityName);

    // Data Management
    UFUNCTION(BlueprintCallable, Category = "Narrative|Data")
    void SaveStoryProgress();

    UFUNCTION(BlueprintCallable, Category = "Narrative|Data")
    void LoadStoryProgress();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnStoryStateChanged OnStoryStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
    FOnDialogueEnded OnDialogueEnded;

protected:
    // Core story data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    FStoryProgress StoryProgress;

    // Dialogue data table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Data")
    class UDataTable* DialogueDataTable;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative|State")
    FDialogueEntry CurrentDialogue;

    // Narrative configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    float ConsciousnessGrowthRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Config")
    TMap<EStoryState, FString> StoryStateDescriptions;

private:
    void InitializeStoryStateDescriptions();
    bool CheckDialogueRequirements(const FDialogueEntry& DialogueEntry) const;
    void ProcessDialogueEffects(const FDialogueEntry& DialogueEntry);
};