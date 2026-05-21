#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> Conditions;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
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
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
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
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueEntry(const FNarr_DialogueEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterStoryBeat(const FNarr_StoryBeat& Beat);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetActiveDialogues() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrationAudio(const FString& AudioPath, float Volume = 1.0f);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_StoryBeat> StoryBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FString> ActiveDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentStoryBeat;

private:
    void InitializeDefaultDialogues();
    void InitializeStoryBeats();
    bool CheckConditions(const TArray<FString>& Conditions) const;
    void ProcessDialogueSequence(const TArray<FString>& Sequence);
};

#include "NarrativeManager.generated.h"