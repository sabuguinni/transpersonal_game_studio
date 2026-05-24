#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_CrowdQuestManager.generated.h"

class ACrowd_MassSimulationManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCrowdCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionProgress;

    FQuest_CrowdObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Hunt;
        TargetLocation = FVector::ZeroVector;
        RequiredRadius = 1000.0f;
        RequiredCrowdCount = 5;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        CompletionProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_CrowdObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MissionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardExperience;

    FQuest_CrowdMission()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        Status = EQuest_MissionStatus::NotStarted;
        MissionStartTime = 0.0f;
        MissionDuration = 600.0f;
        RewardExperience = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CrowdQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_CrowdQuestManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartCrowdMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCrowdMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateCrowdObjective(const FString& MissionID, const FString& ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsCrowdMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_CrowdMission> GetActiveCrowdMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateHerdingMission(const FVector& StartLocation, const FVector& TargetLocation, int32 RequiredHerdSize);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateCrowdEvacuationMission(const FVector& DangerZone, const FVector& SafeZone, float EvacuationRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CreateTribalGatheringMission(const FVector& GatheringLocation, int32 RequiredAttendees);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckCrowdObjectiveProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterCrowdManager(ACrowd_MassSimulationManager* CrowdManager);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_CrowdMission> ActiveCrowdMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_CrowdMission> CompletedCrowdMissions;

    UPROPERTY()
    ACrowd_MassSimulationManager* CrowdManagerRef;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ObjectiveCheckInterval;

private:
    void InitializeDefaultCrowdMissions();
    void UpdateMissionTimers();
    bool CheckObjectiveCompletion(const FQuest_CrowdObjective& Objective);
    int32 GetCrowdCountInRadius(const FVector& Location, float Radius);
    void OnMissionCompleted(const FQuest_CrowdMission& Mission);

    FTimerHandle ObjectiveCheckTimer;
};