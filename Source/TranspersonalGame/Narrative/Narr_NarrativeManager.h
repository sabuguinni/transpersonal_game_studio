#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    ENarr_NarrativeEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    float EventPriority;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    TArray<FString> AudioClips;

    FNarr_NarrativeEvent()
    {
        EventID = TEXT("");
        EventTitle = TEXT("");
        EventDescription = TEXT("");
        EventType = ENarr_NarrativeEventType::Discovery;
        EventPriority = 1.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString ChapterID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> UnlockedAreas;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    float SurvivalDays;

    FNarr_StoryProgress()
    {
        ChapterID = TEXT("Chapter01_Awakening");
        CurrentObjective = 0;
        SurvivalDays = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_NarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_NarrativeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(const FString& EventID, AActor* SourceActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventAvailable(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryProgress(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_StoryProgress GetCurrentStoryProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterEventCompletion(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_NarrativeEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadNarrativeData(UDataTable* NarrativeDataTable);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeAudio(const FString& AudioClipID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryChapter(const FString& ChapterID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FNarr_NarrativeEvent> NarrativeEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    FNarr_StoryProgress CurrentStoryProgress;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    float EventCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TMap<FString, float> EventLastTriggered;

private:
    void InitializeDefaultEvents();
    bool CheckEventConditions(const FNarr_NarrativeEvent& Event) const;
    void ProcessEventTrigger(const FNarr_NarrativeEvent& Event, AActor* SourceActor);
};