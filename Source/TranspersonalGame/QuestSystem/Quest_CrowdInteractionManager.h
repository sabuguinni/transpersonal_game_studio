#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Quest_CrowdInteractionManager.generated.h"

// Forward declarations
class UCrowd_PathfindingManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_CrowdMissionObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuestObjectiveType::Gather;
        TargetLocation = FVector::ZeroVector;
        RequiredCrowdCount = 5;
        CompletionRadius = 500.0f;
        TimeLimit = 300.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdInfluenceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float InfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float InfluenceStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowdBehaviorType TargetBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector InfluenceDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsActive;

    FQuest_CrowdInfluenceData()
    {
        InfluenceRadius = 1000.0f;
        InfluenceStrength = 0.7f;
        TargetBehavior = ECrowdBehaviorType::Following;
        InfluenceDirection = FVector::ForwardVector;
        bIsActive = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdObjectiveProgress, const FString&, ObjectiveID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdObjectiveCompleted, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCrowdInfluenceChanged, FVector, Location, float, Radius, float, Strength);

/**
 * Quest_CrowdInteractionManager
 * 
 * Manages quest objectives that involve crowd interaction and manipulation.
 * Handles evacuation missions, crowd guidance, and large-scale NPC coordination.
 * Integrates with the crowd pathfinding system for realistic group behavior.
 */
UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_CrowdInteractionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdInteractionManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core crowd mission management
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void StartCrowdMission(const FString& MissionID, const TArray<FQuest_CrowdMissionObjective>& Objectives);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void EndCrowdMission(const FString& MissionID, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    bool IsCrowdMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    float GetCrowdMissionProgress(const FString& MissionID) const;

    // Player crowd influence
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void SetPlayerCrowdInfluence(const FQuest_CrowdInfluenceData& InfluenceData);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void UpdatePlayerInfluenceLocation(FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void DisablePlayerCrowdInfluence();

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void AddCrowdObjective(const FString& MissionID, const FQuest_CrowdMissionObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void RemoveCrowdObjective(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    bool IsObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID) const;

    // Crowd monitoring
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    int32 GetCrowdCountInArea(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    TArray<FVector> GetCrowdPositionsInArea(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void GuideCrowdToLocation(FVector TargetLocation, float InfluenceRadius);

    // Emergency scenarios
    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void TriggerCrowdEvacuation(FVector DangerZone, float DangerRadius, FVector SafeZone);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void CreateCrowdGatheringPoint(FVector Location, float Radius, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Quest Crowd")
    void DisperseCrowdFromArea(FVector Center, float Radius);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnCrowdObjectiveProgress OnCrowdObjectiveProgress;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnCrowdObjectiveCompleted OnCrowdObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnCrowdInfluenceChanged OnCrowdInfluenceChanged;

protected:
    // Mission data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest State")
    TMap<FString, TArray<FQuest_CrowdMissionObjective>> ActiveCrowdMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest State")
    TMap<FString, float> MissionStartTimes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest State")
    FQuest_CrowdInfluenceData CurrentPlayerInfluence;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float ObjectiveCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float MaxInfluenceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    int32 MaxSimultaneousMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    bool bEnableDebugVisualization;

    // References
    UPROPERTY()
    UCrowd_PathfindingManager* CrowdPathfindingManager;

    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

private:
    // Internal methods
    void UpdateCrowdObjectives(float DeltaTime);
    void CheckObjectiveCompletion(const FString& MissionID, FQuest_CrowdMissionObjective& Objective);
    void UpdatePlayerInfluence(float DeltaTime);
    void DrawDebugInfo() const;
    
    float LastObjectiveCheck;
    float LastInfluenceUpdate;
};