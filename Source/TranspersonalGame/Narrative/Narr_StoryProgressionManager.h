#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaRising     UMETA(DisplayName = "Alpha Rising"),
    Survival        UMETA(DisplayName = "Survival")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PrerequisiteEvents;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bCompleted = false;
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
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetStoryProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetCurrentPhaseDescription() const;

private:
    UPROPERTY()
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY()
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY()
    int32 CompletedEventsCount;

    void InitializeStoryEvents();
    bool ArePrerequisitesMet(const FNarr_StoryEvent& Event) const;
};