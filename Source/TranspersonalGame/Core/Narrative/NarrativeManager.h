#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString VoiceAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        VoiceAssetPath = TEXT("");
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_DialogueEntry> ChapterDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryChapter()
    {
        ChapterName = TEXT("Untitled Chapter");
        ChapterDescription = FText::FromString(TEXT("Chapter description"));
        bIsCompleted = false;
    }
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Inactive,
    WaitingForPlayer,
    DisplayingDialogue,
    WaitingForResponse,
    Completed
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANarrativeManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Story progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 CurrentStoryChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 TotalStoryChapters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FNarr_StoryChapter> StoryChapters;

    // Dialogue system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueState CurrentDialogueState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FNarr_DialogueEntry ActiveDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueInteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> TribalElderDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> HunterDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> ScoutDialogues;

    // Narrative events
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    TArray<FString> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Events")
    TArray<FString> ActiveNarrativeFlags;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& NPCName, const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryChapter();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetNarrativeFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FText GetCurrentChapterDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogueOptions() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueResponse(int32 ResponseIndex);

private:
    void InitializeStoryChapters();
    void InitializeDialogues();
    void UpdateDialogueState(float DeltaTime);
    
    float DialogueTimer;
    int32 CurrentDialogueIndex;
};