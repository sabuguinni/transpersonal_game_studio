#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_SystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RequiredItems;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        QuestType = EQuestType::Hunt;
        TargetCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector MissionArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float MissionRadius;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        MissionType = EQuestType::Hunt;
        Difficulty = EQuestDifficulty::Easy;
        TimeLimit = 0.0f;
        bIsActive = false;
        bIsCompleted = false;
        MissionArea = FVector::ZeroVector;
        MissionRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateMission(const FString& MissionID, const FString& Title, const FString& Description, EQuestType Type, EQuestDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ActivateMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsMissionCompleted(const FString& MissionID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddObjectiveToMission(const FString& MissionID, const FString& ObjectiveID, const FString& Description, EQuestType Type, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteObjective(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID) const;

    // Location-based Objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SetObjectiveLocation(const FString& MissionID, const FString& ObjectiveID, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckLocationObjective(const FString& MissionID, const FString& ObjectiveID, const FVector& PlayerLocation) const;

    // Crowd Integration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateCrowdBasedMission(const FString& MissionID, const FString& Title, const FVector& CrowdArea, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateCrowdMissionProgress(const FString& MissionID, int32 CrowdDensity);

    // Dynamic Quest Generation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateHuntMission(const FString& TargetSpecies, const FVector& HuntingGrounds, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateGatherMission(const FString& ResourceType, const FVector& GatheringArea, int32 ResourceCount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GenerateExplorationMission(const FVector& ExploreLocation, float ExploreRadius);

    // Quest Data Access
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_ObjectiveData> GetMissionObjectives(const FString& MissionID) const;

    // System Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ResetAllQuests();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_MissionData> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_MissionData> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> AvailableMissionTemplates;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    int32 MaxActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    float QuestUpdateInterval;

    // Internal Methods
    void ValidateMissionObjectives(FQuest_MissionData& Mission);
    void CheckMissionCompletion(const FString& MissionID);
    void GenerateRandomMissionID();
    void CleanupExpiredMissions();

private:
    FTimerHandle QuestUpdateTimer;
    int32 NextMissionIDCounter;
};

#include "Quest_SystemManager.generated.h"