#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_PrehistoricMissionManager.generated.h"

// Mission Types for Prehistoric Survival
UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt           UMETA(DisplayName = "Hunt Mission"),
    Explore        UMETA(DisplayName = "Exploration Mission"),
    Craft          UMETA(DisplayName = "Crafting Mission"),
    Defend         UMETA(DisplayName = "Defense Mission"),
    Rescue         UMETA(DisplayName = "Rescue Mission"),
    Migration      UMETA(DisplayName = "Migration Mission"),
    Territory      UMETA(DisplayName = "Territory Mission"),
    Survival       UMETA(DisplayName = "Survival Mission")
};

// Mission Status Tracking
UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    NotStarted     UMETA(DisplayName = "Not Started"),
    Active         UMETA(DisplayName = "Active"),
    Completed      UMETA(DisplayName = "Completed"),
    Failed         UMETA(DisplayName = "Failed"),
    Abandoned      UMETA(DisplayName = "Abandoned")
};

// Mission Priority Levels
UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Low            UMETA(DisplayName = "Low Priority"),
    Medium         UMETA(DisplayName = "Medium Priority"),
    High           UMETA(DisplayName = "High Priority"),
    Critical       UMETA(DisplayName = "Critical Priority"),
    Emergency      UMETA(DisplayName = "Emergency Priority")
};

// Mission Objective Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetType;

    FQuest_MissionObjective()
    {
        ObjectiveDescription = TEXT("");
        bIsCompleted = false;
        bIsOptional = false;
        TargetCount = 1;
        CurrentCount = 0;
        TargetType = TEXT("");
    }
};

// Mission Reward Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ResourceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString UnlockReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    bool bUnlocksNewArea;

    FQuest_MissionReward()
    {
        ExperiencePoints = 0;
        ResourceReward = 0;
        UnlockReward = TEXT("");
        bUnlocksNewArea = false;
    }
};

// Complete Mission Data Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PrehistoricMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_MissionReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RepeatCount;

    FQuest_PrehistoricMission()
    {
        MissionID = TEXT("");
        MissionTitle = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_MissionType::Survival;
        Status = EQuest_MissionStatus::NotStarted;
        Priority = EQuest_MissionPriority::Medium;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 1000.0f;
        bIsRepeatable = false;
        RepeatCount = 0;
    }
};

/**
 * Prehistoric Mission Manager
 * Manages all mission types for prehistoric survival gameplay
 * Handles mission creation, tracking, completion, and rewards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_PrehistoricMissionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_PrehistoricMissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void CreateMission(const FQuest_PrehistoricMission& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    bool AbandonMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Management")
    void UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Mission Queries")
    TArray<FQuest_PrehistoricMission> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Queries")
    TArray<FQuest_PrehistoricMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission Queries")
    FQuest_PrehistoricMission GetMissionByID(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Queries")
    TArray<FQuest_PrehistoricMission> GetMissionsByType(EQuest_MissionType MissionType) const;

    UFUNCTION(BlueprintCallable, Category = "Mission Queries")
    bool IsMissionCompleted(const FString& MissionID) const;

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateHuntMission(const FString& TargetSpecies, const FVector& HuntArea, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateExplorationMission(const FVector& ExploreLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateCraftingMission(const FString& ItemToCraft, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateDefenseMission(const FVector& DefenseLocation, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateRescueMission(const FVector& RescueLocation, const FString& RescueTarget);

    // Mission Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Events")
    void OnMissionStarted(const FQuest_PrehistoricMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Events")
    void OnMissionCompleted(const FQuest_PrehistoricMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Events")
    void OnMissionFailed(const FQuest_PrehistoricMission& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Events")
    void OnObjectiveUpdated(const FQuest_PrehistoricMission& Mission, int32 ObjectiveIndex);

protected:
    // Mission Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_PrehistoricMission> AllMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FString> ActiveMissionIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FString> CompletedMissionIDs;

    // Mission Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float LastMissionGenerationTime;

    // Mission Templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Templates")
    TArray<FQuest_PrehistoricMission> HuntMissionTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Templates")
    TArray<FQuest_PrehistoricMission> ExplorationMissionTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Templates")
    TArray<FQuest_PrehistoricMission> CraftingMissionTemplates;

    // Helper Functions
    FString GenerateUniqueMissionID();
    bool CheckMissionPrerequisites(const FQuest_PrehistoricMission& Mission) const;
    void ProcessMissionRewards(const FQuest_PrehistoricMission& Mission);
    void UpdateMissionTimers(float DeltaTime);
    void CheckLocationBasedObjectives();
    void AutoGenerateMissions();
};