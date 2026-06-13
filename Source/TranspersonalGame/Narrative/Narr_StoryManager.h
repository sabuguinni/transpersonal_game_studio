#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribeContact    UMETA(DisplayName = "Tribe Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaChallenge  UMETA(DisplayName = "Alpha Challenge"),
    Survival        UMETA(DisplayName = "Survival")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventTitle = FText::GetEmpty();
        EventDescription = FText::GetEmpty();
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsStoryEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryProgress(const FString& ProgressKey, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetStoryProgress(const FString& ProgressKey) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, int32> StoryProgressMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    UDataTable* StoryEventDataTable;

private:
    void LoadStoryEvents();
    void InitializeDefaultEvents();
    bool CheckEventConditions(const FNarr_StoryEvent& Event) const;
};