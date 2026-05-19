#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryChapter : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaChallenge  UMETA(DisplayName = "Alpha Challenge"),
    Exodus          UMETA(DisplayName = "Exodus")
};

UENUM(BlueprintType)
enum class ENarr_DialogueState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Respectful      UMETA(DisplayName = "Respectful")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryChapter CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 ChapterProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, int32> CharacterRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasMetTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bHasKilledAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 SurvivalDays;

    FNarr_StoryProgress()
    {
        CurrentChapter = ENarr_StoryChapter::Awakening;
        ChapterProgress = 0;
        bHasMetTribe = false;
        bHasKilledAlpha = false;
        SurvivalDays = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueState RequiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextNodeIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestRelated;

    FNarr_DialogueNode()
    {
        RequiredState = ENarr_DialogueState::Neutral;
        bIsQuestRelated = false;
    }
};

/**
 * Story Manager - Controls narrative progression and dialogue system
 * Tracks player choices, story events, and character relationships
 */
UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceChapter(ENarr_StoryChapter NewChapter);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool HasCompletedEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryChapter GetCurrentChapter() const;

    // Character relationships
    UFUNCTION(BlueprintCallable, Category = "Story")
    void ModifyRelationship(const FString& CharacterName, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetRelationshipLevel(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_DialogueState GetDialogueState(const FString& CharacterName) const;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& CharacterName, const FString& DialogueTreeID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueNode GetCurrentDialogueNode() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsInDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    // Story state queries
    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetSurvivalDays() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void IncrementSurvivalDay();

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CanAccessChapter(ENarr_StoryChapter Chapter) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    FNarr_StoryProgress StoryProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TMap<FString, FNarr_DialogueNode> DialogueDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString CurrentDialogueTreeID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString CurrentNodeID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    bool bInDialogue;

private:
    void LoadDialogueDatabase();
    void SaveStoryProgress();
    void LoadStoryProgress();
    ENarr_DialogueState CalculateDialogueState(const FString& CharacterName) const;
};