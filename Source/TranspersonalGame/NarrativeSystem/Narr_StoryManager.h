#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening = 0,
    FirstHunt = 1,
    TribalContact = 2,
    TerritoryExploration = 3,
    AlphaEncounter = 4,
    PackLeadership = 5,
    ValleyMastery = 6
};

UENUM(BlueprintType)
enum class ENarr_StoryEvent : uint8
{
    None = 0,
    PlayerSpawned = 1,
    FirstDinosaurSighted = 2,
    FirstKill = 3,
    TribalMeeting = 4,
    TerritoryDiscovered = 5,
    AlphaDefeated = 6,
    PackFormed = 7,
    ValleyControlled = 8
};

USTRUCT(BlueprintType)
struct FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<ENarr_StoryEvent> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 TerritoriesDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float SurvivalTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTribalContactMade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bAlphaDefeated;

    FNarr_StoryProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        DinosaurKills = 0;
        TerritoriesDiscovered = 0;
        SurvivalTime = 0.0f;
        bTribalContactMade = false;
        bAlphaDefeated = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_StoryTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryEvent TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString DialogueSequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bOnlyOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTriggered;

    FNarr_StoryTrigger()
    {
        TriggerEvent = ENarr_StoryEvent::None;
        DialogueSequenceName = TEXT("");
        NarrativeText = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bOnlyOnce = true;
        bTriggered = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(ENarr_StoryEvent Event);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool HasCompletedEvent(ENarr_StoryEvent Event) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void IncrementDinosaurKills();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void IncrementTerritoriesDiscovered();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateSurvivalTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryProgress GetStoryProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetTribalContact(bool bContactMade);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetAlphaDefeated(bool bDefeated);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetCurrentPhaseDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryTrigger(const FNarr_StoryTrigger& Trigger);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FNarr_StoryProgress CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryTrigger> StoryTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<ENarr_StoryPhase, FString> PhaseDescriptions;

private:
    void InitializeStoryTriggers();
    void InitializePhaseDescriptions();
    void ProcessStoryTrigger(const FNarr_StoryTrigger& Trigger);
    void CheckPhaseAdvancement();
};