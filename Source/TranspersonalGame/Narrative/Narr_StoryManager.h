#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryChapter : uint8
{
    Awakening,      // Player starts alone, learns basic survival
    Discovery,      // First dinosaur encounters, territory awareness
    Adaptation,     // Tool crafting, shelter building
    Confrontation,  // Major predator encounters
    Mastery,        // Advanced survival, leadership
    Legacy          // End game content
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryChapter RequiredChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        RequiredChapter = ENarr_StoryChapter::Awakening;
        bIsCompleted = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceToChapter(ENarr_StoryChapter NewChapter);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryChapter GetCurrentChapter() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID) const;

    // Story Events
    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    // Narrative Triggers
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerNarrativeEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetChapterDescription(ENarr_StoryChapter Chapter) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryChapter CurrentChapter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> CompletedEvents;

private:
    void InitializeDefaultStoryEvents();
    bool ArePrerequisitesMet(const FNarr_StoryEvent& Event) const;
    void BroadcastChapterChange(ENarr_StoryChapter NewChapter);
};