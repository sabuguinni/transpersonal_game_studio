#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_CrowdIntegrationManager.generated.h"

class UCrowd_MassEntityManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    EQuest_ObjectiveType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    float EventRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    float EventDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Event")
    bool bIsActive;

    FQuest_CrowdEvent()
    {
        EventID = TEXT("DefaultEvent");
        TriggerType = EQuest_ObjectiveType::Gather;
        RequiredCrowdSize = 10;
        EventRadius = 1000.0f;
        EventLocation = FVector::ZeroVector;
        EventDuration = 60.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Response")
    FString ResponseID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Response")
    ECrowd_BehaviorType NewBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Response")
    float ResponseIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Response")
    float ResponseDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Response")
    int32 AffectedEntityCount;

    FQuest_CrowdResponse()
    {
        ResponseID = TEXT("DefaultResponse");
        NewBehavior = ECrowd_BehaviorType::Wandering;
        ResponseIntensity = 1.0f;
        ResponseDuration = 30.0f;
        AffectedEntityCount = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdIntegrationManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    TArray<FQuest_CrowdEvent> ActiveCrowdEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    TArray<FQuest_CrowdResponse> CrowdResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    UCrowd_MassEntityManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    float EventCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    int32 MaxSimultaneousEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Integration")
    bool bEnableCrowdQuestIntegration;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void TriggerCrowdEvent(const FString& EventID, FVector Location, int32 CrowdSize);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void EndCrowdEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void SetCrowdBehaviorForQuest(const FString& QuestID, ECrowd_BehaviorType NewBehavior, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    bool IsCrowdEventActive(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    int32 GetCrowdSizeInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void CreateQuestGatheringEvent(FVector Location, int32 RequiredPeople, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void CreateQuestEscortEvent(FVector StartLocation, FVector EndLocation, int32 EscortSize);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void CreateQuestDefenseEvent(FVector DefenseLocation, float DefenseRadius, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd Integration")
    void UpdateQuestCrowdObjectives();

private:
    void CheckActiveEvents();
    void ProcessCrowdResponses();
    void ValidateCrowdManager();

    FTimerHandle EventCheckTimer;
    float LastEventCheck;
};