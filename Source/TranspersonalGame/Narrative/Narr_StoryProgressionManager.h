#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryChapter : uint8
{
    Awakening,      // Tutorial - learning basic survival
    FirstHunt,      // First dinosaur encounter/kill
    TribalContact,  // Meeting other survivors
    PackLeader,     // Becoming leader of a group
    TerritoryWars,  // Conflicts with other tribes
    Exodus,         // Major migration/escape
    NewLands,       // Discovering new territories
    Legacy          // End game - establishing permanent settlement
};

UENUM(BlueprintType)
enum class ENarr_StoryTrigger : uint8
{
    PlayerAction,       // Player completes specific action
    TimeElapsed,        // Story advances after time period
    LocationDiscovered, // Player reaches specific location
    NPCInteraction,     // Conversation with key NPC
    QuestCompleted,     // Major quest milestone
    SurvivalThreshold,  // Player reaches survival milestone
    CombatVictory,      // Defeats major threat
    ResourceGathered    // Collects critical resources
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryChapter RequiredChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString AudioCueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bIsRepeatable;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        RequiredChapter = ENarr_StoryChapter::Awakening;
        TriggerType = ENarr_StoryTrigger::PlayerAction;
        TriggerCondition = TEXT("");
        NarrativeText = TEXT("");
        AudioCueID = TEXT("");
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_ChapterProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter Progress")
    ENarr_StoryChapter CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter Progress")
    float ChapterProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter Progress")
    TArray<FString> AvailableEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter Progress")
    FDateTime ChapterStartTime;

    FNarr_ChapterProgress()
    {
        CurrentChapter = ENarr_StoryChapter::Awakening;
        ChapterProgress = 0.0f;
        ChapterStartTime = FDateTime::Now();
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryProgressionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Story progression methods
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void AdvanceChapter(ENarr_StoryChapter NewChapter);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool CanTriggerEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    ENarr_StoryChapter GetCurrentChapter() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    float GetChapterProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetAvailableEvents() const;

    // Event registration
    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void RegisterStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void UnregisterStoryEvent(const FString& EventID);

    // Chapter management
    UFUNCTION(BlueprintCallable, Category = "Chapter Management")
    void SetChapterProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Chapter Management")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Chapter Management")
    void MarkEventCompleted(const FString& EventID);

    // Narrative queries
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetChapterNarrative(ENarr_StoryChapter Chapter) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetEventNarrative(const FString& EventID) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story State")
    FNarr_ChapterProgress CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story Events")
    TMap<FString, FNarr_StoryEvent> RegisteredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chapter Narratives")
    TMap<ENarr_StoryChapter, FString> ChapterNarratives;

private:
    void InitializeChapterNarratives();
    void InitializeDefaultEvents();
    void UpdateAvailableEvents();
    bool CheckTriggerCondition(const FNarr_StoryEvent& Event) const;
};