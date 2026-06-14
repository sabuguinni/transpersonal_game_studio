#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasMetElderThok;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasEnteredValley;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasFoundAncientRuins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurKillCount;

    FNarr_StoryState()
    {
        CurrentChapter = 1;
        bHasMetElderThok = false;
        bHasEnteredValley = false;
        bHasFoundAncientRuins = false;
        DinosaurKillCount = 0;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStory(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsQuestActive(const FString& QuestName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsQuestCompleted(const FString& QuestName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void StartQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetCurrentChapterTitle() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventName, const FVector& Location);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FNarr_StoryState StoryState;

    UPROPERTY(EditDefaultsOnly, Category = "Story")
    TMap<int32, FString> ChapterTitles;

    void InitializeChapterTitles();
    void HandleStoryTrigger(const FString& TriggerName, const FVector& Location);
};