#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Quest_MissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionTitle = TEXT("");
        MissionDescription = TEXT("");
        ExperienceReward = 0;
        bIsMainQuest = false;
        MissionLocation = FVector::ZeroVector;
        MissionRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ActiveMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Mission")
    TArray<bool> ObjectiveStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Mission")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Mission")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Mission")
    bool bIsFailed;

    FQuest_ActiveMission()
    {
        MissionID = TEXT("");
        StartTime = 0.0f;
        bIsCompleted = false;
        bIsFailed = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_MissionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool UpdateObjective(const FString& MissionID, int32 ObjectiveIndex, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FQuest_ActiveMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    TArray<FString> GetCompletedMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    bool IsMissionCompleted(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void LoadMissionDatabase();

    UFUNCTION(BlueprintCallable, Category = "Mission Manager")
    void CreateSurvivalMissions();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    class UDataTable* MissionDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    TArray<FQuest_ActiveMission> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    TArray<FString> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission State")
    TMap<FString, FQuest_MissionData> MissionDatabase;

private:
    void InitializeSurvivalMissions();
    void RegisterMissionCallbacks();
    bool ValidateMissionPrerequisites(const FString& MissionID) const;
    void BroadcastMissionUpdate(const FString& MissionID, const FString& UpdateType);
};