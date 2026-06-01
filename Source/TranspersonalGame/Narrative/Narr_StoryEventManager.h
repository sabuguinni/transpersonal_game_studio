#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Narr_StoryEventManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryEventType : uint8
{
    Discovery,
    Encounter,
    Survival,
    Migration,
    Conflict,
    Achievement,
    Environmental,
    Character
};

UENUM(BlueprintType)
enum class ENarr_StoryEventPriority : uint8
{
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_StoryEventPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> Consequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    float TriggerDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    int32 MaxOccurrences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    float CooldownTime;

    FNarr_StoryEvent()
    {
        EventType = ENarr_StoryEventType::Discovery;
        Priority = ENarr_StoryEventPriority::Medium;
        TriggerDelay = 0.0f;
        bIsRepeatable = false;
        MaxOccurrences = 1;
        CooldownTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveStoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Event")
    FNarr_StoryEvent StoryEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Event")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Event")
    int32 OccurrenceCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Event")
    float LastTriggerTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Event")
    bool bIsActive;

    FNarr_ActiveStoryEvent()
    {
        TimeRemaining = 0.0f;
        OccurrenceCount = 0;
        LastTriggerTime = 0.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_StoryEventData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Events")
    TMap<ENarr_StoryEventType, float> TypeWeights;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryEventTriggered, const FNarr_StoryEvent&, StoryEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryEventCompleted, const FNarr_StoryEvent&, StoryEvent);

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryEventManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UPROPERTY(BlueprintAssignable, Category = "Story Events")
    FOnStoryEventTriggered OnStoryEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Story Events")
    FOnStoryEventCompleted OnStoryEventCompleted;

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void LoadStoryEventData(UNarr_StoryEventData* EventData);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    bool CanTriggerEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    TArray<FNarr_StoryEvent> GetAvailableEvents();

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    TArray<FNarr_ActiveStoryEvent> GetActiveEvents();

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void SetEventPrerequisite(const FString& EventID, const FString& Prerequisite);

    UFUNCTION(BlueprintCallable, Category = "Story Events")
    void ClearEventPrerequisite(const FString& EventID, const FString& Prerequisite);

protected:
    UPROPERTY()
    TArray<FNarr_StoryEvent> LoadedStoryEvents;

    UPROPERTY()
    TArray<FNarr_ActiveStoryEvent> ActiveStoryEvents;

    UPROPERTY()
    TArray<FString> CompletedEventIDs;

    UPROPERTY()
    TArray<FString> GlobalPrerequisites;

    void ProcessActiveEvents(float DeltaTime);
    void CheckEventTriggers();
    bool ArePrerequisitesMet(const FNarr_StoryEvent& Event);
    void ExecuteEventConsequences(const FNarr_StoryEvent& Event);
};