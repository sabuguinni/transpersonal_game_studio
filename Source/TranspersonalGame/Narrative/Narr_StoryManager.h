#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player awakens in prehistoric world
    FirstHunt,      // Learning to hunt small prey
    TribalContact,  // Meeting other survivors
    TerritoryWar,   // Conflict over resources
    AlphaChallenge, // Facing apex predators
    Mastery         // Becoming apex survivor
};

UENUM(BlueprintType)
enum class ENarr_StoryEvent : uint8
{
    PlayerSpawn,
    FirstKill,
    FirstDeath,
    TribalMeeting,
    ResourceDiscovery,
    TerritoryMarked,
    AlphaEncounter,
    SurvivalMilestone
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryState : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 KillCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DeathCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTribalContactMade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTerritoryEstablished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bAlphaDefeated;

    FNarr_StoryState()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        SurvivalDays = 0;
        KillCount = 0;
        DeathCount = 0;
        bTribalContactMade = false;
        bTerritoryEstablished = false;
        bAlphaDefeated = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryTrigger : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryEvent TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString TriggerDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bAdvancePhase;

    FNarr_StoryTrigger()
    {
        TriggerEvent = ENarr_StoryEvent::PlayerSpawn;
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bAdvancePhase = false;
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
    void TriggerStoryEvent(ENarr_StoryEvent Event, AActor* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryState GetStoryState() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateSurvivalStats(int32 DaysAlive, int32 Kills, int32 Deaths);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AddObjective(const FString& ObjectiveID, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FString> GetActiveObjectives() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void LoadStoryData(UDataTable* StoryTable);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SaveStoryProgress();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void LoadStoryProgress();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FNarr_StoryState CurrentStoryState;

    UPROPERTY()
    UDataTable* StoryDataTable;

    UPROPERTY()
    TMap<ENarr_StoryEvent, FNarr_StoryTrigger> StoryTriggers;

private:
    void ProcessStoryTrigger(const FNarr_StoryTrigger& Trigger, AActor* Instigator);
    bool CheckPhaseRequirements(ENarr_StoryPhase Phase);
    void InitializeDefaultObjectives();
    FString GetPhaseDescription(ENarr_StoryPhase Phase);
};

#include "Narr_StoryManager.generated.h"