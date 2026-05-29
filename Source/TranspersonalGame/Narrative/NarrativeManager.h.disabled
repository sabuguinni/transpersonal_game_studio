#pragma once
// DISABLED: // DISABLED: #include "TranspersonalGameSharedTypes.h"

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeTypes.h"
#include "GameplayTagContainer.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        Duration = 3.0f;
        bIsNarration = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueEntry> Dialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDialogue(const FNarr_DialogueEntry& DialogueEntry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetEventCompleted(const FString& EventID, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryEvent> GetActiveEvents() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsDialogueActive;

private:
    void InitializeDefaultEvents();
    void LoadStoryData();
    void SaveStoryProgress();
};