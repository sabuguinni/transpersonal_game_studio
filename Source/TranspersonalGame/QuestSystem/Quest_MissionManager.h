#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest_MissionObjective.h"
#include "Engine/Engine.h"
#include "Quest_MissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<UQuest_MissionObjective*> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsMainMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString RewardDescription;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        bIsMainMission = false;
        bIsActive = false;
        bIsCompleted = false;
        Priority = 1;
        RewardDescription = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FString&, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, const FString&, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, const FString&, ObjectiveID);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_MissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnMissionStarted OnMissionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void AbandonMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionCompleted(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FQuest_MissionData> GetCompletedMissions() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 IncrementAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CompleteObjective(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void SetObjectiveTargetLocation(const FString& MissionID, const FString& ObjectiveID, const FVector& Location, float Radius = 500.0f);

    // Survival Mission Templates
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateHuntMission(const FString& DinosaurType, int32 TargetCount, const FVector& HuntArea);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateGatherMission(const FString& ResourceType, int32 TargetCount, float TimeLimit = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateExplorationMission(const FVector& TargetLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateSurvivalMission(float SurvivalTime, const FVector& SafeZone);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateDefenseMission(const FVector& DefensePoint, float DefenseRadius, float Duration);

    // System Updates
    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void UpdateMissionTimers(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CheckPlayerLocationObjectives(const FVector& PlayerLocation);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mission Manager")
    void CreateTestMissions();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Mission Manager")
    void ClearAllMissions();

protected:
    UPROPERTY()
    TArray<FQuest_MissionData> AllMissions;

    UPROPERTY()
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY()
    TArray<FQuest_MissionData> CompletedMissions;

    void CheckMissionCompletion(FQuest_MissionData& Mission);
    FQuest_MissionData* FindMissionByID(const FString& MissionID);
    UQuest_MissionObjective* FindObjectiveByID(FQuest_MissionData& Mission, const FString& ObjectiveID);
    FString GenerateUniqueMissionID();
};