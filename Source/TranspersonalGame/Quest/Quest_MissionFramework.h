#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Quest_MissionFramework.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt           UMETA(DisplayName = "Hunt Mission"),
    Gather         UMETA(DisplayName = "Gather Resources"),
    Explore        UMETA(DisplayName = "Exploration"),
    Craft          UMETA(DisplayName = "Crafting"),
    Rescue         UMETA(DisplayName = "Rescue Mission"),
    Defense        UMETA(DisplayName = "Defense Mission"),
    Discovery      UMETA(DisplayName = "Discovery Mission")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive       UMETA(DisplayName = "Inactive"),
    Available      UMETA(DisplayName = "Available"),
    Active         UMETA(DisplayName = "Active"),
    Completed      UMETA(DisplayName = "Completed"),
    Failed         UMETA(DisplayName = "Failed"),
    Abandoned      UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Beginner       UMETA(DisplayName = "Beginner"),
    Novice         UMETA(DisplayName = "Novice"),
    Experienced    UMETA(DisplayName = "Experienced"),
    Expert         UMETA(DisplayName = "Expert"),
    Master         UMETA(DisplayName = "Master")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetActorTag;

    FQuest_MissionObjective()
    {
        ObjectiveDescription = TEXT("Default Objective");
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetActorTag = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ResourcePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString UnlockableContent;

    FQuest_MissionReward()
    {
        ExperiencePoints = 100;
        ResourcePoints = 50;
        UnlockableContent = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
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
    EQuest_MissionStatus MissionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_MissionReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius;

    FQuest_MissionData()
    {
        MissionID = TEXT("MISSION_001");
        MissionTitle = TEXT("Survival Mission");
        MissionDescription = TEXT("Complete survival objectives");
        MissionType = EQuest_MissionType::Hunt;
        MissionStatus = EQuest_MissionStatus::Available;
        Difficulty = EQuest_Difficulty::Beginner;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        MissionLocation = FVector::ZeroVector;
        MissionRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_MissionFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool CreateMission(const FQuest_MissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool AbandonMission(const FString& MissionID);

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool UpdateObjective(const FString& MissionID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool CompleteObjective(const FString& MissionID, int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    TArray<FQuest_MissionObjective> GetMissionObjectives(const FString& MissionID);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    TArray<FQuest_MissionData> GetAvailableMissions();

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    TArray<FQuest_MissionData> GetActiveMissions();

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    FQuest_MissionData GetMissionData(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    bool IsMissionCompleted(const FString& MissionID);

    // Dynamic Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    FQuest_MissionData GenerateHuntMission(const FVector& Location, const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    FQuest_MissionData GenerateGatherMission(const FVector& Location, const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Mission Framework")
    FQuest_MissionData GenerateExplorationMission(const FVector& Location, float ExplorationRadius);

    // Mission Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Framework")
    void OnMissionStarted(const FQuest_MissionData& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Framework")
    void OnMissionCompleted(const FQuest_MissionData& Mission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission Framework")
    void OnObjectiveUpdated(const FString& MissionID, const FQuest_MissionObjective& Objective);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    TArray<FQuest_MissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    TArray<FQuest_MissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    TArray<FQuest_MissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Framework")
    float LastMissionGenerationTime;

private:
    void UpdateMissionTimers(float DeltaTime);
    void CheckMissionCompletion();
    void GenerateRandomMissions();
    FString GenerateUniqueMissionID();
    bool ValidateMissionData(const FQuest_MissionData& MissionData);
};