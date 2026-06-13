#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Quest_MissionObjectiveManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Target,
    Collect_Resources,
    Explore_Area,
    Survive_Duration,
    Craft_Item,
    Defend_Location,
    Escort_NPC,
    Reach_Location
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveStatus : uint8
{
    Inactive,
    Active,
    Completed,
    Failed
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    TArray<FString> RequiredItems;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Title = "";
        Description = "";
        Type = EQuest_ObjectiveType::Hunt_Target;
        Status = EQuest_ObjectiveStatus::Inactive;
        TargetCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsMainQuest;

    FQuest_MissionData()
    {
        MissionID = "";
        MissionName = "";
        MissionDescription = "";
        ExperienceReward = 100;
        bIsMainQuest = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_MissionObjectiveManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_MissionObjectiveManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    UDataTable* MissionDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Mission System")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission System")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(BlueprintReadOnly, Category = "Mission System")
    FQuest_MissionData CurrentMainMission;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsObjectiveComplete(const FString& MissionID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_ObjectiveData> GetActiveObjectives();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData GetMissionByID(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void RegisterItemCollection(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void RegisterEnemyKill(const FString& EnemyType);

    UFUNCTION(BlueprintPure, Category = "Mission System")
    int32 GetActiveMissionCount() const { return ActiveMissions.Num(); }

    UFUNCTION(BlueprintPure, Category = "Mission System")
    int32 GetCompletedMissionCount() const { return CompletedMissions.Num(); }

protected:
    void InitializeDefaultMissions();
    bool CheckObjectiveCompletion(FQuest_ObjectiveData& Objective);
    void OnObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID);
    void OnMissionCompleted(const FString& MissionID);
};

#include "Quest_MissionObjectiveManager.generated.h"