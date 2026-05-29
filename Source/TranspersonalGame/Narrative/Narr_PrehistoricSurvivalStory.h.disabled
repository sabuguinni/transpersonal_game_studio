#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_PrehistoricSurvivalStory.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryChapter : uint8
{
    Prologue        UMETA(DisplayName = "Prologue: Stranded"),
    Chapter1        UMETA(DisplayName = "Chapter 1: First Contact"),
    Chapter2        UMETA(DisplayName = "Chapter 2: Learning the Land"),
    Chapter3        UMETA(DisplayName = "Chapter 3: Predator Territory"),
    Chapter4        UMETA(DisplayName = "Chapter 4: Tribal Encounters"),
    Chapter5        UMETA(DisplayName = "Chapter 5: The Great Migration"),
    Chapter6        UMETA(DisplayName = "Chapter 6: Alpha Predator"),
    Epilogue        UMETA(DisplayName = "Epilogue: Master Survivor")
};

UENUM(BlueprintType)
enum class ENarr_CharacterArc : uint8
{
    Survivor        UMETA(DisplayName = "Lone Survivor"),
    Explorer        UMETA(DisplayName = "Territory Explorer"),
    Hunter          UMETA(DisplayName = "Predator Hunter"),
    Researcher      UMETA(DisplayName = "Behavioral Researcher"),
    Leader          UMETA(DisplayName = "Tribal Leader")
};

UENUM(BlueprintType)
enum class ENarr_StoryTone : uint8
{
    Desperate       UMETA(DisplayName = "Desperate Survival"),
    Cautious        UMETA(DisplayName = "Cautious Exploration"),
    Confident       UMETA(DisplayName = "Confident Mastery"),
    Scientific      UMETA(DisplayName = "Scientific Discovery"),
    Heroic          UMETA(DisplayName = "Heroic Leadership")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    ENarr_StoryChapter Chapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> RequiredObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> UnlockedContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bIsCompleted;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = FText::GetEmpty();
        BeatDescription = FText::GetEmpty();
        Chapter = ENarr_StoryChapter::Prologue;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText CharacterRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> VoicelineIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterArc CharacterArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsUnlocked;

    FNarr_CharacterProfile()
    {
        CharacterID = TEXT("");
        CharacterName = FText::GetEmpty();
        CharacterRole = FText::GetEmpty();
        BackgroundStory = FText::GetEmpty();
        CharacterArc = ENarr_CharacterArc::Survivor;
        bIsUnlocked = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_StoryChapter CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_StoryTone CurrentTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float SurvivalMastery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 SuccessfulHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> CompletedStoryBeats;

    FNarr_NarrativeContext()
    {
        CurrentChapter = ENarr_StoryChapter::Prologue;
        CurrentTone = ENarr_StoryTone::Desperate;
        SurvivalMastery = 0.0f;
        DinosaurEncounters = 0;
        SuccessfulHunts = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_PrehistoricSurvivalStory : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_PrehistoricSurvivalStory();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void InitializeStoryFramework();

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void AdvanceStoryChapter(ENarr_StoryChapter NewChapter);

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    bool CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void UpdateNarrativeContext(float NewSurvivalMastery, int32 NewEncounters, int32 NewHunts);

    // Character development
    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    void UnlockCharacter(const FString& CharacterID);

    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    void ProgressCharacterArc(ENarr_CharacterArc NewArc);

    UFUNCTION(BlueprintCallable, Category = "Character Arc")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID);

    // Story queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story State")
    ENarr_StoryChapter GetCurrentChapter() const { return NarrativeContext.CurrentChapter; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story State")
    ENarr_StoryTone GetCurrentTone() const { return NarrativeContext.CurrentTone; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story State")
    float GetSurvivalMastery() const { return NarrativeContext.SurvivalMastery; }

    UFUNCTION(BlueprintCallable, Category = "Story State")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats();

    UFUNCTION(BlueprintCallable, Category = "Story State")
    TArray<FNarr_CharacterProfile> GetUnlockedCharacters();

    // Narrative events
    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void TriggerDinosaurEncounterStory(const FString& DinosaurType, bool bSurvived);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void TriggerResourceDiscoveryStory(const FString& ResourceType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void TriggerSurvivalMilestoneStory(float MasteryLevel);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Data")
    TArray<FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    TArray<FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Context")
    FNarr_NarrativeContext NarrativeContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Settings")
    class UDataTable* StoryBeatsDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
    class UDataTable* CharacterProfilesDataTable;

private:
    void LoadStoryBeatsFromDataTable();
    void LoadCharacterProfilesFromDataTable();
    void CreateDefaultStoryBeats();
    void CreateDefaultCharacterProfiles();
    void UpdateStoryTone();
    void CheckChapterProgression();
    
    UPROPERTY()
    class UNarr_DialogueManager* DialogueManager;
    
    UPROPERTY()
    class UNarr_VoicelineManager* VoicelineManager;
};