#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    ESurvivalStat RequiredStat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float ElapsedTime;

    FQuest_SurvivalObjective()
    {
        ObjectiveID = "";
        Description = "";
        RequiredStat = ESurvivalStat::Health;
        TargetValue = 100.0f;
        CurrentValue = 0.0f;
        bIsCompleted = false;
        TimeLimit = 300.0f; // 5 minutes default
        ElapsedTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    EBiomeType RequiredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float MissionStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FString> RewardItems;

    FQuest_SurvivalMission()
    {
        MissionID = "";
        Title = "";
        Description = "";
        RequiredBiome = EBiomeType::Savana;
        bIsActive = false;
        bIsCompleted = false;
        MissionStartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    // Predefined Missions
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateBasicSurvivalMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateWaterSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateFoodSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CreateShelterSurvivalMission();

    // Survival Stat Monitoring
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void MonitorSurvivalStats();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CheckObjectiveCompletion();

    // Mission Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionStarted(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnMissionCompleted(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Missions")
    void OnObjectiveCompleted(const FQuest_SurvivalObjective& Objective);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Survival Missions")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Missions")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Missions")
    TMap<FString, FQuest_SurvivalMission> MissionDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Missions")
    float LastStatCheckTime;

private:
    void InitializeMissionDatabase();
    FQuest_SurvivalMission* FindMissionByID(const FString& MissionID);
    void ProcessMissionTimeouts();
    void UpdateMissionTimers(float DeltaTime);
};