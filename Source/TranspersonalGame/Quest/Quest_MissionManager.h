#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Quest_MissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ESurvivalDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ObjectiveIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        Description = TEXT("");
        Difficulty = ESurvivalDifficulty::Easy;
        bIsActive = false;
        bIsCompleted = false;
        RewardExperience = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveName = TEXT("");
        Description = TEXT("");
        QuestType = EQuestType::Hunt;
        TargetID = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UQuest_MissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionCompleted(const FString& MissionID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void UpdateObjectiveProgress(const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    // Quest Creation
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateHuntMission(const FString& DinosaurSpecies, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateGatherMission(const FString& ResourceType, int32 RequiredAmount);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateExploreMission(const FVector& TargetLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateSurvivalMission(float Duration, ESurvivalDifficulty Difficulty);

    // Data Access
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FQuest_ObjectiveData> GetMissionObjectives(const FString& MissionID) const;

protected:
    UPROPERTY()
    TMap<FString, FQuest_MissionData> Missions;

    UPROPERTY()
    TMap<FString, FQuest_ObjectiveData> Objectives;

    UPROPERTY()
    TArray<FString> ActiveMissionIDs;

    void InitializeDefaultMissions();
    void CreateDefaultHuntMissions();
    void CreateDefaultGatherMissions();
    void CreateDefaultExploreMissions();
    
    FString GenerateUniqueID(const FString& Prefix) const;
    void BroadcastMissionUpdate(const FString& MissionID);
    void BroadcastObjectiveUpdate(const FString& ObjectiveID);
};

#include "Quest_MissionManager.generated.h"