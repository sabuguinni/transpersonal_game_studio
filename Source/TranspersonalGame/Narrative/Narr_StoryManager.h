#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    ApexChallenge   UMETA(DisplayName = "Apex Challenge"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"),
    LegendaryHunter UMETA(DisplayName = "Legendary Hunter")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CompletionCount;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
        bIsRepeatable = false;
        CompletionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENPCRole CharacterRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 RelationshipLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsTrusted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsRival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> CompletedInteractions;

    FNarr_CharacterArc()
    {
        CharacterName = TEXT("");
        CharacterRole = ENPCRole::Gatherer;
        RelationshipLevel = 0;
        bIsTrusted = false;
        bIsRival = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryPhase(ENarr_StoryPhase NewPhase);

    // Story Events
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableStoryEvents() const;

    // Character Relationships
    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateCharacterRelationship(const FString& CharacterName, int32 RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Character")
    int32 GetCharacterRelationshipLevel(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetCharacterTrusted(const FString& CharacterName, bool bTrusted);

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool IsCharacterTrusted(const FString& CharacterName) const;

    // Player Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Progress")
    void RecordPlayerAction(const FString& ActionType, const FString& ActionDetails);

    UFUNCTION(BlueprintCallable, Category = "Progress")
    int32 GetPlayerActionCount(const FString& ActionType) const;

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void UpdateSurvivalMilestone(const FString& MilestoneType, int32 Value);

    // Narrative Context
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetContextualNarration(const FString& Situation) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerEnvironmentalNarration(const FString& LocationTag);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TMap<FString, FNarr_CharacterArc> CharacterArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TMap<FString, int32> PlayerActionCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TMap<FString, int32> SurvivalMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FString> ContextualNarrations;

private:
    void InitializeStoryEvents();
    void InitializeCharacterArcs();
    void InitializeNarrations();
    void CheckStoryPhaseProgression();
    bool ValidateStoryPhaseAdvancement(ENarr_StoryPhase TargetPhase) const;
};