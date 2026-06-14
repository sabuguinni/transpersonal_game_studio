#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,
    FirstHunt,
    TribalContact,
    TerritoryWars,
    AlphaStatus
};

USTRUCT(BlueprintType)
struct FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FString EventID;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FText EventDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    float CompletionTime;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventDescription = FText::GetEmpty();
        bIsCompleted = false;
        CompletionTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetActiveEvents() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    float GameStartTime;

private:
    void InitializeStoryEvents();
    void OnPhaseChanged(ENarr_StoryPhase OldPhase, ENarr_StoryPhase NewPhase);
};