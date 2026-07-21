#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_SurvivalStoryFramework.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryChapter : uint8
{
    Awakening = 0,      // Player awakens alone, learns basic survival
    FirstContact,       // Encounters first tribal NPCs
    TribalIntegration,  // Joins tribe, learns customs
    TerritoryExpansion, // Explores new regions, faces new threats
    AlphaChallenge,     // Becomes tribal leader through survival skills
    LegacyBuilding      // Establishes lasting tribal civilization
};

UENUM(BlueprintType)
enum class ENarr_StoryMood : uint8
{
    Desperate = 0,      // Life-threatening situations
    Cautious,           // Exploring unknown territory
    Hopeful,            // Finding resources or allies
    Triumphant,         // Overcoming major challenges
    Contemplative       // Quiet moments of reflection
};

USTRUCT(BlueprintType)
struct FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryChapter Chapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryMood Mood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        Chapter = ENarr_StoryChapter::Awakening;
        Mood = ENarr_StoryMood::Desperate;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_TribalLore
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString OriginStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    TArray<FString> SacredPlaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    TArray<FString> TabooActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    TArray<FString> SurvivalWisdom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString LeadershipTradition;

    FNarr_TribalLore()
    {
        TribeName = TEXT("Stone-Walker Tribe");
        OriginStory = TEXT("Born from the great mountain's shadow, we learned to walk with stone-tooth beasts");
        SacredPlaces.Add(TEXT("The First Cave"));
        SacredPlaces.Add(TEXT("Blood-Stone Circle"));
        TabooActions.Add(TEXT("Never hunt alone at night"));
        TabooActions.Add(TEXT("Never waste water in dry season"));
        SurvivalWisdom.Add(TEXT("Sharp-claw tracks mean death follows"));
        SurvivalWisdom.Add(TEXT("Green water brings sickness"));
        LeadershipTradition = TEXT("Only those who survive the Great Hunt may lead");
    }
};

/**
 * Survival Story Framework - Manages the overarching narrative that connects
 * individual survival missions into a cohesive prehistoric survival saga.
 * Tracks player progression through tribal integration and leadership.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_SurvivalStoryFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_SurvivalStoryFramework();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void InitializeStoryFramework();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryChapter GetCurrentChapter() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryBeat GetCurrentStoryBeat() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats() const;

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Story")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool ShouldUnlockQuest(const FString& QuestID) const;

    // Character development
    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdatePlayerReputation(int32 ReputationChange);

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetPlayerReputation() const { return PlayerReputation; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CanBecomeTribalLeader() const;

    // Tribal lore
    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_TribalLore GetTribalLore() const { return TribalLore; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetContextualNarration(const FString& Context) const;

    // Story mood and atmosphere
    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryMood GetCurrentMood() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryMood(ENarr_StoryMood NewMood);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString CurrentStoryBeatID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryChapter CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 PlayerReputation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FNarr_TribalLore TribalLore;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> CompletedQuests;

private:
    void CreateDefaultStoryBeats();
    void CheckChapterProgression();
    FString GenerateContextualDialogue(const FString& Context, ENarr_StoryMood Mood) const;
};