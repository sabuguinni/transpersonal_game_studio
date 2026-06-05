#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 ChapterOrder;

    FNarr_StoryChapter()
    {
        ChapterName = TEXT("");
        ChapterDescription = TEXT("");
        bIsCompleted = false;
        ChapterOrder = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> AvailableEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float RelationshipLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsAlive;

    FNarr_CharacterArc()
    {
        CharacterID = TEXT("");
        CharacterName = TEXT("");
        RelationshipLevel = 0.0f;
        bIsAlive = true;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryProgressionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void InitializeStorySystem();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteChapter(const FString& ChapterID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsChapterUnlocked(const FString& ChapterID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryChapter GetCurrentChapter() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryChapter> GetAvailableChapters() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateCharacterRelationship(const FString& CharacterID, float DeltaRelationship);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void TriggerCharacterEvent(const FString& CharacterID, const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Character")
    FNarr_CharacterArc GetCharacterArc(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SaveStoryProgress();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void LoadStoryProgress();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryChapter> StoryChapters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FNarr_CharacterArc> CharacterArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CurrentChapterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentChapterIndex;

private:
    void SetupDefaultStoryChapters();
    void SetupDefaultCharacterArcs();
    bool CheckChapterRequirements(const FNarr_StoryChapter& Chapter) const;
    void UnlockNewQuests(const FNarr_StoryChapter& Chapter);
};