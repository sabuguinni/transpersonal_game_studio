#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 Priority;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        bIsCompleted = false;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetActiveStoryEvents();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TMap<FString, FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    bool bIsDialoguePlaying;

private:
    void InitializeStoryEvents();
    void LoadDialogueDatabase();
};