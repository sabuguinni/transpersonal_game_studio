#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    ESurvivalStat RequiredStat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FVector RequiredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float LocationRadius;

    FQuest_SurvivalObjective()
    {
        ObjectiveName = TEXT("Survive");
        RequiredStat = ESurvivalStat::Health;
        TargetValue = 100.0f;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        RequiredLocation = FVector::ZeroVector;
        LocationRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    EBiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    int32 RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float MissionStartTime;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Survival Test");
        MissionDescription = TEXT("Survive the harsh prehistoric world");
        TargetBiome = EBiomeType::Savanna;
        RewardExperience = 100;
        bIsActive = false;
        bIsCompleted = false;
        MissionStartTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Missions")
    TArray<FQuest_SurvivalMission> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Missions")
    FQuest_SurvivalMission CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Missions")
    bool bHasActiveMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Missions")
    float MissionCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Missions")
    int32 CompletedMissionsCount;

    FTimerHandle MissionUpdateTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void StartMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void UpdateMissionProgress();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool CheckObjectiveCompletion(const FQuest_SurvivalObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void GenerateRandomMission(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    TArray<FQuest_SurvivalMission> GetAvailableMissions() const { return AvailableMissions; }

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    FQuest_SurvivalMission GetCurrentMission() const { return CurrentMission; }

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    bool HasActiveMission() const { return bHasActiveMission; }

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    void InitializeDefaultMissions();

    UFUNCTION(BlueprintCallable, Category = "Survival Missions")
    FVector GetBiomeCenter(EBiomeType BiomeType);

private:
    void CreateSavannaSurvivalMission();
    void CreateForestSurvivalMission();
    void CreateDesertSurvivalMission();
    void CreateSwampSurvivalMission();
    void CreateMountainSurvivalMission();
    
    bool IsPlayerInLocation(const FVector& Location, float Radius);
    float GetPlayerSurvivalStat(ESurvivalStat StatType);
};