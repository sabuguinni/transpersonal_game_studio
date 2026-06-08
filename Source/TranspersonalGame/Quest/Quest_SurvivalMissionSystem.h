#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalMissionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString TargetActorTag;

    FQuest_MissionObjective()
    {
        ObjectiveText = TEXT("Unknown Objective");
        bIsCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
        TargetActorTag = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    FQuest_SurvivalMission()
    {
        MissionID = TEXT("MISSION_001");
        MissionTitle = TEXT("Survive the Day");
        MissionDescription = TEXT("Basic survival mission");
        bIsActive = false;
        bIsCompleted = false;
        TimeLimit = 600.0f; // 10 minutes
        ElapsedTime = 0.0f;
        ExperienceReward = 100;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalMissionSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntingMission(const FString& DinosaurType, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatheringMission(const FString& ResourceType, int32 RequiredAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExplorationMission(const FString& LocationName, float DiscoveryRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterKill(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterLocationDiscovered(const FString& LocationName);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float MissionCheckInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    float LastMissionCheck;

private:
    void CheckMissionTimeouts(float DeltaTime);
    void CheckObjectiveCompletion();
    FQuest_SurvivalMission* FindMissionByID(const FString& MissionID);
    void InitializeDefaultMissions();
    FString GenerateUniqueMissionID();
};