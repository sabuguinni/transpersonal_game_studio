#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Quest_MissionSystem.generated.h"

// Forward declarations
class AQuest_MissionActor;
class UQuest_ResourceGatheringManager;
class UCrowd_MassSimulationManager;

UENUM(BlueprintType)
enum class EQuest_MissionType : uint8
{
    None = 0,
    Gather = 1,
    Hunt = 2,
    Explore = 3,
    Escort = 4,
    Defend = 5,
    Craft = 6,
    Trade = 7
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Inactive = 0,
    Active = 1,
    Completed = 2,
    Failed = 3,
    Abandoned = 4
};

UENUM(BlueprintType)
enum class EQuest_MissionPriority : uint8
{
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3,
    Emergency = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    EQuest_MissionType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    int32 TargetQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Objective")
    bool bIsCompleted;

    FQuest_MissionObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        ObjectiveType = EQuest_MissionType::None;
        TargetQuantity = 1;
        CurrentProgress = 0;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString MissionID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionStatus MissionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    EQuest_MissionPriority MissionPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_MissionObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FVector MissionLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float MissionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    bool bRequiresCrowdInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    bool bTriggersEmergencyResponse;

    FQuest_MissionData()
    {
        MissionID = TEXT("");
        MissionName = TEXT("");
        MissionDescription = TEXT("");
        MissionType = EQuest_MissionType::None;
        MissionStatus = EQuest_MissionStatus::Inactive;
        MissionPriority = EQuest_MissionPriority::Medium;
        TimeLimit = 0.0f;
        TimeRemaining = 0.0f;
        MissionLocation = FVector::ZeroVector;
        MissionRadius = 1000.0f;
        ExperienceReward = 100;
        bRequiresCrowdInteraction = false;
        bTriggersEmergencyResponse = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionStatusChanged, const FString&, MissionID, EQuest_MissionStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, MissionID, const FString&, ObjectiveID, int32, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, const FString&, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, const FString&, MissionID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_MissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_MissionSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateMission(const FQuest_MissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool StartMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool CompleteMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool FailMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool AbandonMission(const FString& MissionID);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressDelta);

    // Mission Queries
    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    TArray<FQuest_MissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    TArray<FQuest_MissionData> GetMissionsByType(EQuest_MissionType MissionType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FQuest_MissionData GetMissionData(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    bool IsMissionActive(const FString& MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    int32 GetActiveMissionCount() const;

    // Crowd Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    void TriggerCrowdEmergencyResponse(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    void NotifyCrowdOfMissionProgress(const FString& MissionID, EQuest_MissionStatus Status);

    // Mission Templates
    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateGatheringMission(const FVector& Location, const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateHuntingMission(const FVector& Location, const FString& TargetSpecies, int32 TargetCount);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateExplorationMission(const FVector& TargetLocation, float ExplorationRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateEscortMission(const FVector& StartLocation, const FVector& EndLocation, const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest Mission System")
    FString CreateDefenseMission(const FVector& DefenseLocation, float DefenseRadius, float Duration);

    // Event Delegates
    UPROPERTY(BlueprintAssignable, Category = "Quest Mission System Events")
    FOnMissionStatusChanged OnMissionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Mission System Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Mission System Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Mission System Events")
    FOnMissionFailed OnMissionFailed;

protected:
    // Mission Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_MissionData> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_MissionData> CompletedMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    int32 MissionCounter;

    // System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    UQuest_ResourceGatheringManager* ResourceManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    UCrowd_MassSimulationManager* CrowdManager;

    // Mission Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Mission System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FTimerHandle> MissionTimers;

    // Internal Methods
    void UpdateMissionTimers();
    void OnMissionTimeExpired(const FString& MissionID);
    bool ValidateMissionObjectives(const FQuest_MissionData& MissionData) const;
    void BroadcastMissionStatusChange(const FString& MissionID, EQuest_MissionStatus NewStatus);
    FString GenerateUniqueMissionID();
};

#include "Quest_MissionSystem.generated.h"