#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Crowd_QuestCrowdManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_QuestEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    FString EventID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    FVector EventLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    float EventRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    ECrowd_BehaviorType EventBehavior;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    int32 RequiredCrowdSize;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Event")
    bool bIsActive;

    FCrowd_QuestEvent()
    {
        EventID = TEXT("");
        EventLocation = FVector::ZeroVector;
        EventRadius = 1000.0f;
        EventBehavior = ECrowd_BehaviorType::Gathering;
        RequiredCrowdSize = 20;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_QuestResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest Response")
    FString ResponseID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Response")
    TArray<FVector> GatheringPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Response")
    ECrowd_BehaviorType ResponseBehavior;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Response")
    float ResponseDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Response")
    int32 ParticipantCount;

    FCrowd_QuestResponse()
    {
        ResponseID = TEXT("");
        ResponseBehavior = ECrowd_BehaviorType::Gathering;
        ResponseDuration = 300.0f;
        ParticipantCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_QuestCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_QuestCrowdManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void RegisterQuestEvent(const FCrowd_QuestEvent& QuestEvent);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void TriggerQuestEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void CompleteQuestEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    FCrowd_QuestResponse GetQuestResponse(const FString& EventID);

    // Crowd Behavior for Quests
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void SpawnQuestCrowd(const FVector& Location, int32 CrowdSize, ECrowd_BehaviorType Behavior);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void MoveQuestCrowd(const FString& EventID, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void DismissQuestCrowd(const FString& EventID);

    // Story Integration
    UFUNCTION(BlueprintCallable, Category = "Story Integration")
    void CreateStoryGathering(const FVector& Location, const FString& StoryContext);

    UFUNCTION(BlueprintCallable, Category = "Story Integration")
    void TriggerStoryReaction(const FString& StoryEvent, float IntensityLevel);

    UFUNCTION(BlueprintCallable, Category = "Story Integration")
    void SetStoryMood(ECrowd_BehaviorType MoodType, float Duration);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Events")
    TArray<FCrowd_QuestEvent> ActiveQuestEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Events")
    TArray<FCrowd_QuestResponse> QuestResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float QuestEventRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    int32 MaxQuestCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float QuestResponseTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Settings")
    float StoryMoodDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Settings")
    TArray<FVector> StoryGatheringPoints;

private:
    void ProcessQuestEvents(float DeltaTime);
    void UpdateQuestCrowds(float DeltaTime);
    void HandleStoryIntegration(float DeltaTime);
    
    FCrowd_QuestEvent* FindQuestEvent(const FString& EventID);
    void SpawnQuestNPCs(const FCrowd_QuestEvent& QuestEvent);
    void UpdateNPCBehavior(const FCrowd_QuestEvent& QuestEvent);
};