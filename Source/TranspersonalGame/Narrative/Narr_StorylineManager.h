#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_StorylineManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,
    FirstHunt,
    TribalContact,
    TerritoryEstablishment,
    PackFormation,
    AlphaChallenge,
    TribalLeadership,
    LegendaryHunter
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> UnlockedQuests;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    int32 HuntsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    int32 TribalContactsMade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    int32 TerritoriesEstablished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    bool bHasPackAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    bool bIsTribalLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    TArray<FString> CompletedEvents;

    FNarr_PlayerProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        HuntsCompleted = 0;
        TribalContactsMade = 0;
        TerritoriesEstablished = 0;
        bHasPackAllies = false;
        bIsTribalLeader = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StorylineManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Manager")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Manager")
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Manager")
    bool bStorylineActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return PlayerProgress.CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    FNarr_PlayerProgress GetPlayerProgress() const { return PlayerProgress; }

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void RegisterHuntCompletion();

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void RegisterTribalContact();

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void RegisterTerritoryEstablishment();

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void SetPackAlliesStatus(bool bHasAllies);

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void SetTribalLeaderStatus(bool bIsLeader);

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    TArray<FString> GetAvailableDialogues() const;

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Storyline Manager")
    void InitializeStoryEvents();

private:
    void CheckPhaseProgression();
    void UnlockContentForPhase(ENarr_StoryPhase Phase);
};

#include "Narr_StorylineManager.generated.h"