#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletionFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = TEXT("");
        BeatDescription = TEXT("");
        bIsActive = false;
        bIsCompleted = false;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioPath = TEXT("");
        Duration = 3.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatActive(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(const FString& DialogueID, AActor* Speaker);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryBeat> GetActiveStoryBeats() const;

protected:
    UPROPERTY()
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY()
    TMap<FString, FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY()
    TMap<FString, bool> StoryFlags;

    void LoadStoryData();
    void InitializeMainStoryBeats();
};