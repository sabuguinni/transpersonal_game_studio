#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Quest_CrowdIntegrationManager.generated.h"

// Forward declarations
class UCrowd_MassEntityManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EscortObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    float EscortRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    TArray<ATargetPoint*> WaypointRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    float CompletionReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Escort")
    bool bIsActive;

    FQuest_EscortObjective()
    {
        ObjectiveName = TEXT("Escort Mission");
        RequiredCrowdSize = 10;
        EscortRadius = 500.0f;
        CompletionReward = 100.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdControlObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    float ControlRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    int32 MaxCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    float CompletionReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Crowd Control")
    bool bIsActive;

    FQuest_CrowdControlObjective()
    {
        ObjectiveName = TEXT("Crowd Control Mission");
        ControlRadius = 1000.0f;
        MaxCrowdDensity = 50;
        TimeLimit = 300.0f;
        CompletionReward = 150.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdEvacuationObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    FVector DangerZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    float DangerZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    FVector SafeZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    int32 RequiredEvacuees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    float CompletionReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Evacuation")
    bool bIsActive;

    FQuest_CrowdEvacuationObjective()
    {
        ObjectiveName = TEXT("Evacuation Mission");
        DangerZoneCenter = FVector::ZeroVector;
        DangerZoneRadius = 1500.0f;
        SafeZoneCenter = FVector(0, 0, 0);
        RequiredEvacuees = 25;
        TimeLimit = 600.0f;
        CompletionReward = 200.0f;
        bIsActive = false;
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
    virtual void Tick(float DeltaTime) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    // Quest objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_EscortObjective> EscortObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_CrowdControlObjective> CrowdControlObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Objectives")
    TArray<FQuest_CrowdEvacuationObjective> EvacuationObjectives;

    // Crowd integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    UCrowd_MassEntityManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    float CrowdInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    int32 CurrentCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    float CrowdDensityThreshold;

    // Quest state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    bool bQuestSystemActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    int32 ActiveQuestCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    float TotalQuestRewards;

    // Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timers")
    float EscortMissionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timers")
    float CrowdControlTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timers")
    float EvacuationTimer;

public:
    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartEscortMission(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartCrowdControlMission(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartEvacuationMission(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteQuest(const FString& QuestName, float RewardAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void FailQuest(const FString& QuestName);

    // Crowd interaction functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    int32 GetNearbyCrowdCount();

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    float CalculateCrowdDensity();

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void TriggerCrowdPanic();

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void DirectCrowdToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Interaction")
    void SetCrowdBehaviorZone(const FVector& Center, float Radius, int32 BehaviorType);

    // Quest validation functions
    UFUNCTION(BlueprintCallable, Category = "Quest Validation")
    bool ValidateEscortProgress(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Validation")
    bool ValidateCrowdControlProgress(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Validation")
    bool ValidateEvacuationProgress(int32 ObjectiveIndex);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Quest Utilities")
    void UpdateQuestMarkers();

    UFUNCTION(BlueprintCallable, Category = "Quest Utilities")
    void ResetAllQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest Utilities")
    FString GetQuestStatusReport();

protected:
    // Internal quest processing
    void ProcessEscortObjectives(float DeltaTime);
    void ProcessCrowdControlObjectives(float DeltaTime);
    void ProcessEvacuationObjectives(float DeltaTime);

    // Quest completion handlers
    void OnEscortMissionComplete(int32 ObjectiveIndex);
    void OnCrowdControlMissionComplete(int32 ObjectiveIndex);
    void OnEvacuationMissionComplete(int32 ObjectiveIndex);

    // Crowd event handlers
    void OnCrowdDensityChanged(float NewDensity);
    void OnCrowdPanicTriggered();
    void OnCrowdReachedWaypoint(int32 WaypointIndex);

private:
    // Internal state tracking
    TArray<float> EscortProgressTracking;
    TArray<float> CrowdControlProgressTracking;
    TArray<float> EvacuationProgressTracking;
    
    float LastCrowdUpdateTime;
    int32 CompletedQuestCount;
    float SystemStartTime;
};