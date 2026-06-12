#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

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
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CompletionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedFlags;

    FNarr_StoryEvent()
    {
        EventID = TEXT("default_event");
        EventTitle = TEXT("Unknown Event");
        EventDescription = FText::FromString(TEXT("An event occurred"));
        bCompleted = false;
        CompletionTime = 0.0f;
    }
};

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TribalContact   UMETA(DisplayName = "Tribal Contact"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    GreatMigration  UMETA(DisplayName = "Great Migration"),
    Survival        UMETA(DisplayName = "Survival"),
    Mastery         UMETA(DisplayName = "Mastery")
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> ActiveFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float GameStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 SuccessfulHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 ResourcesGathered;

public:
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool HasStoryFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RemoveStoryFlag(const FString& FlagName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetActiveEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterDinosaurEncounter();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterSuccessfulHunt();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterResourceGathered();

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetSurvivalTime() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetPhaseDescription() const;

private:
    void InitializeStoryEvents();
    void CheckPhaseProgression();
    FString GetPhaseDescriptionInternal(ENarr_StoryPhase Phase) const;
};