#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstSteps      UMETA(DisplayName = "First Steps"),
    LearningToHunt  UMETA(DisplayName = "Learning to Hunt"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    PackLeader      UMETA(DisplayName = "Pack Leader"),
    ApexChallenge   UMETA(DisplayName = "Apex Challenge"),
    Mastery         UMETA(DisplayName = "Mastery")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> CompletionRewards;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    float EventWeight;

    FNarr_StoryEvent()
    {
        EventID = "";
        EventTitle = "";
        EventDescription = "";
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
        EventWeight = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 DinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 DaysAlive;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 LocationsDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 ToolsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 SheltersBuilt;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    TArray<FString> UnlockedAreas;

    FNarr_PlayerProgress()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        DinosaurKills = 0;
        DaysAlive = 0;
        LocationsDiscovered = 0;
        ToolsCrafted = 0;
        SheltersBuilt = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdatePlayerProgress(const FString& ProgressType, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetPhaseNarration(ENarr_StoryPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool HasCompletedEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_PlayerProgress GetPlayerProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterCustomEvent(const FNarr_StoryEvent& CustomEvent);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Story")
    TMap<ENarr_StoryPhase, FString> PhaseNarrations;

    void LoadDefaultStoryEvents();
    void LoadPhaseNarrations();
    bool CheckEventConditions(const FNarr_StoryEvent& Event) const;
    void ProcessEventCompletion(const FNarr_StoryEvent& Event);
    ENarr_StoryPhase CalculateNextPhase() const;
};