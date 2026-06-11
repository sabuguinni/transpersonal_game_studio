#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribeContact    UMETA(DisplayName = "Tribe Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaChallenge  UMETA(DisplayName = "Alpha Challenge"),
    Mastery         UMETA(DisplayName = "Mastery")
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
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventDescription = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
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
    FString GetCurrentPhaseDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryEvent(const FNarr_StoryEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetStoryProgress() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    int32 CompletedEventsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    int32 TotalEventsCount;

private:
    void InitializeStoryEvents();
    bool ArePrerequisitesMet(const FNarr_StoryEvent& Event) const;
};