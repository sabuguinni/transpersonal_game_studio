#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Narr_DialogueSystem.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,
    FirstHunt,
    TribalContact,
    TerritoryEstablishment,
    AlphaChallenge,
    PackLeadership
};

UENUM(BlueprintType)
enum class ENarr_StoryEvent : uint8
{
    PlayerSpawn,
    FirstDinosaurEncounter,
    FirstKill,
    DiscoverWater,
    MeetNPC,
    CompleteQuest,
    DinosaurDeath,
    PlayerDeath
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_StoryPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority;

    FNarr_StoryNode()
    {
        NodeID = TEXT("");
        Phase = ENarr_StoryPhase::Awakening;
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        bIsCompleted = false;
        Priority = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RelatedStoryNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelationshipWithPlayer;

    FNarr_CharacterArc()
    {
        CharacterName = TEXT("");
        CharacterType = ENarr_CharacterType::TribalElder;
        BackgroundStory = FText::GetEmpty();
        bIsAlive = true;
        RelationshipWithPlayer = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(ENarr_StoryEvent Event, const FString& EventData = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryNode> GetActiveStoryNodes();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacterArc(const FNarr_CharacterArc& CharacterArc);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterArc GetCharacterArc(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateCharacterRelationship(const FString& CharacterName, float RelationshipDelta);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializePrehistoricStory();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCurrentNarrativeContext();

protected:
    UPROPERTY()
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY()
    TMap<FString, FNarr_StoryNode> StoryNodes;

    UPROPERTY()
    TMap<FString, FNarr_CharacterArc> CharacterArcs;

    UPROPERTY()
    TArray<FString> CompletedEvents;

    UPROPERTY()
    float StoryProgressionScore;

    UPROPERTY()
    UNarr_DialogueSystem* DialogueSystem;

    void CreatePrehistoricStoryNodes();
    void CreateCharacterArcs();
    void CheckStoryProgression();
    bool AreRequirementsMet(const FNarr_StoryNode& Node);
    void TriggerNarrativeSequence(const FString& NodeID);
};