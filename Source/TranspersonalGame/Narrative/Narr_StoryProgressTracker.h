#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressTracker.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedSequences;

    FNarr_StoryEvent()
    {
        EventID = TEXT("default");
        EventDescription = TEXT("Unknown event");
        bIsCompleted = false;
        CompletionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> DiscoveredLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DaysAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float TotalPlayTime;

    FNarr_PlayerProgress()
    {
        DinosaurKills = 0;
        DaysAlive = 0;
        TotalPlayTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_StoryProgressTracker : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_StoryProgressTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    FString CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    int32 CurrentDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    float DayStartTime;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void UnlockDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    bool IsDialogueUnlocked(const FString& DialogueID) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void DiscoverLocation(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void RecordDinosaurKill(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void AdvanceDay();

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    TArray<FString> GetAvailableDialogues() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    FString GetProgressSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void SaveProgress();

    UFUNCTION(BlueprintCallable, Category = "Story Progress")
    void LoadProgress();

private:
    void UpdateDayCounter(float DeltaTime);
    void CheckStoryTriggers();
};