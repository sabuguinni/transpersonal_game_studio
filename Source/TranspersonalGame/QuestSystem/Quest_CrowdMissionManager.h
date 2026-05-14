#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_CrowdMissionManager.generated.h"

class ACrowdSimulationManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TimeLimit;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float ElapsedTime;

    FQuest_CrowdMissionObjective()
    {
        ObjectiveDescription = TEXT("Default Objective");
        ObjectiveType = EQuest_ObjectiveType::Gather;
        RequiredCrowdCount = 10;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        ElapsedTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_CrowdMissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    EQuest_MissionStatus MissionStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    float MissionStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 CompletedObjectives;

    FQuest_CrowdMission()
    {
        MissionName = TEXT("Default Mission");
        MissionDescription = TEXT("A crowd-based survival mission");
        MissionType = EQuest_MissionType::Survival;
        RewardExperience = 100;
        MissionStatus = EQuest_MissionStatus::NotStarted;
        MissionStartTime = 0.0f;
        CompletedObjectives = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdMissionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdMissionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_CrowdMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_CrowdMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    ACrowdSimulationManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Types")
    bool bEnableEvacuationMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Types")
    bool bEnableHuntingMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Types")
    bool bEnableGatheringMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Types")
    bool bEnableDefenseMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TArray<FVector> MissionSpawnLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MissionSpawnRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool StartMission(const FQuest_CrowdMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool CompleteMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void UpdateMissionProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    FQuest_CrowdMission CreateEvacuationMission(FVector DangerLocation, FVector SafeLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    FQuest_CrowdMission CreateHuntingMission(FVector HuntLocation, int32 RequiredHunters);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    FQuest_CrowdMission CreateGatheringMission(FVector ResourceLocation, int32 RequiredGatherers);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    FQuest_CrowdMission CreateDefenseMission(FVector DefenseLocation, float DefenseRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    int32 GetCrowdCountAtLocation(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    bool DirectCrowdToLocation(FVector SourceLocation, FVector TargetLocation, int32 CrowdCount);

    UFUNCTION(BlueprintCallable, Category = "Mission Status")
    TArray<FQuest_CrowdMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Status")
    int32 GetActiveMissionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Status")
    bool HasActiveMissionOfType(EQuest_MissionType MissionType) const;

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void OnPlayerNearMissionArea(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void OnPlayerInfluenceCrowd(FVector InfluenceLocation, float InfluenceRadius);

private:
    void CheckObjectiveCompletion(FQuest_CrowdMissionObjective& Objective);
    void SpawnMissionMarkers(const FQuest_CrowdMission& Mission);
    void CleanupMissionMarkers(int32 MissionIndex);
    void NotifyPlayerMissionUpdate(const FString& Message);

    UPROPERTY()
    TArray<AActor*> MissionMarkers;

    float LastMissionSpawnTime;
    float MissionSpawnCooldown;
};