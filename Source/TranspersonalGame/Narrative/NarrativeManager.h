#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedBeats;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarration(const FString& NarrationID, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeContext(const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentNarrativeContext() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentNarrativeContext;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FString> CompletedStoryBeats;

private:
    void InitializeStoryBeats();
    void LoadDialogueData();
    FNarr_DialogueEntry* FindDialogueEntry(const FString& DialogueID);
};