#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_MissionProgressionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore"),
    Craft       UMETA(DisplayName = "Craft"),
    Defend      UMETA(DisplayName = "Defend"),
    Rescue      UMETA(DisplayName = "Rescue")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Beginner    UMETA(DisplayName = "Beginner"),
    Novice      UMETA(DisplayName = "Novice"),
    Experienced UMETA(DisplayName = "Experienced"),
    Expert      UMETA(DisplayName = "Expert"),
    Legendary   UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_MissionType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    FQuest_MissionObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_MissionType::Gather;
        RequiredCount = 1;
        CurrentCount = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct FQuest_MissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float StaminaBonus;

    FQuest_MissionReward()
    {
        ItemID = TEXT("");
        Quantity = 0;
        ExperiencePoints = 0;
        HealthBonus = 0.0f;
        StaminaBonus = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_MissionReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString NPCGiverID;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        Title = TEXT("");
        Description = TEXT("");
        MissionType = EQuest_MissionType::Gather;
        Difficulty = EQuest_Difficulty::Beginner;
        Status = EQuest_MissionStatus::Inactive;
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        NPCGiverID = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnMissionStatusChanged, const FString&, MissionID, EQuest_MissionStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveCompleted, const FString&, MissionID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnMissionCompleted, const FString&, MissionID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MissionProgressionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MissionProgressionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_MissionData> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    // Mission Creation
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CreateSurvivalMission(const FString& MissionID, EQuest_MissionType Type, EQuest_Difficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool AddObjectiveToMission(const FString& MissionID, const FQuest_MissionObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool SetMissionReward(const FString& MissionID, const FQuest_MissionReward& Reward);

    // Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    float GetMissionProgress(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool IsMissionCompleted(const FString& MissionID) const;

    // Location-based tracking
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CheckLocationObjectives(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FVector> GetActiveObjectiveLocations() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FQuest_OnMissionStatusChanged OnMissionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FQuest_OnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission Events")
    FQuest_OnMissionCompleted OnMissionCompleted;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<FString, FQuest_MissionData> AllMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TArray<FString> ActiveMissionIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Settings")
    float MissionGenerationInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Settings")
    float TimeSinceLastGeneration;

private:
    void InitializeDefaultMissions();
    void GenerateRandomMission();
    void UpdateMissionTimers(float DeltaTime);
    bool CheckObjectiveCompletion(FQuest_MissionObjective& Objective, const FVector& PlayerLocation);
    void BroadcastMissionStatusChange(const FString& MissionID, EQuest_MissionStatus NewStatus);
};