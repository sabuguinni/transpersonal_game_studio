#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "First Awakening"),
    Exploration     UMETA(DisplayName = "World Exploration"),
    FirstHunt       UMETA(DisplayName = "First Successful Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    PackFormation   UMETA(DisplayName = "Pack Formation"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaRise       UMETA(DisplayName = "Rise to Alpha"),
    LegendStatus    UMETA(DisplayName = "Legend Status")
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
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CompletionTime;

    FNarr_StoryEvent()
    {
        EventID = TEXT("default");
        EventTitle = TEXT("Unknown Event");
        EventDescription = FText::FromString(TEXT("An unknown event has occurred."));
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
        CompletionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 Dayssurvived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 TribalAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float TerritorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedEvents;

    FNarr_PlayerProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        DinosaurKills = 0;
        DaysSelected = 0;
        TribalAllies = 0;
        TerritorySize = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_PlayerProgress GetPlayerProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdatePlayerStats(int32 Kills, int32 Days, int32 Allies, float Territory);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FText GetPhaseDescription(ENarr_StoryPhase Phase) const;

protected:
    UPROPERTY()
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY()
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY()
    TArray<FString> ActiveEventQueue;

    void InitializeStoryEvents();
    void CheckPhaseProgression();
    void UnlockNewEvents(const TArray<FString>& EventsToUnlock);
};