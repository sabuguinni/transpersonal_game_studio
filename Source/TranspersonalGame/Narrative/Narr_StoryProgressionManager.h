#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_StoryProgressionManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player starts alone, learns basic survival
    Discovery,      // First encounters with other survivors
    Tribal,         // Joining or forming a tribe
    Expansion,      // Territory growth and resource conflicts
    Mastery,        // Advanced tools, large settlements
    Legacy          // Endgame - leaving mark on the world
};

UENUM(BlueprintType)
enum class ENarr_EventType : uint8
{
    Discovery,      // Finding new locations, resources, creatures
    Survival,       // Overcoming environmental challenges
    Social,         // Interactions with NPCs, tribe dynamics
    Combat,         // Encounters with hostile creatures/tribes
    Achievement,    // Crafting milestones, skill progression
    Narrative       // Story beats, character development
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_EventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> UnlockedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    int32 StoryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FDateTime CompletionTime;

    FNarr_StoryEvent()
    {
        EventID = TEXT("DefaultEvent");
        EventTitle = FText::FromString(TEXT("Unknown Event"));
        EventDescription = FText::FromString(TEXT("An event occurred."));
        EventType = ENarr_EventType::Discovery;
        RequiredPhase = ENarr_StoryPhase::Awakening;
        StoryPoints = 10;
        bIsRepeatable = false;
        CompletionTime = FDateTime::MinValue();
    }
};

USTRUCT(BlueprintType)
struct FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 TotalStoryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> AvailableEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TMap<FString, int32> EventCompletionCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 TribeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    FDateTime GameStartTime;

    FNarr_PlayerProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        TotalStoryPoints = 0;
        SurvivalDays = 0.0f;
        TribeSize = 1;
        GameStartTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    class UDataTable* StoryEventsDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Progress")
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TMap<ENarr_StoryPhase, int32> PhaseRequiredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TMap<FString, FNarr_StoryEvent> LoadedStoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Story System")
    bool bStorySystemInitialized;

public:
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsEventAvailable(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsEventCompleted(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetAvailableEvents();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetCompletedEvents();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    ENarr_StoryPhase GetCurrentStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    int32 GetStoryPoints();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    float GetProgressToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void UpdateSurvivalStats(float DaysElapsed, int32 CurrentTribeSize);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    FNarr_StoryEvent GetStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void RegisterCustomEvent(const FNarr_StoryEvent& CustomEvent);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnStoryEventCompleted(const FNarr_StoryEvent& CompletedEvent);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnStoryPhaseChanged(ENarr_StoryPhase NewPhase, ENarr_StoryPhase PreviousPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "Story Events")
    void OnNewEventUnlocked(const FString& EventID);

private:
    void InitializeDefaultStoryEvents();
    void CheckPhaseProgression();
    void UnlockEventsByCompletion(const FString& CompletedEventID);
    bool CheckEventPrerequisites(const FString& EventID);
    void SaveProgressToFile();
    void LoadProgressFromFile();
};