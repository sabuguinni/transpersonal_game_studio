#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../AI/Crowd/Crowd_MassEntitySubsystem.h"
#include "../AI/Crowd/Crowd_FlockingBehavior.h"
#include "SharedTypes.h"
#include "Quest_CrowdBasedMissionSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    AvoidStampede,
    FollowMigration,
    EscortThroughCrowd,
    ObserveBehavior,
    DisruptFlocking,
    GuideToSafety,
    CountSpecies,
    TrackMovement
};

USTRUCT(BlueprintType)
struct FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdMissionType MissionType = EQuest_CrowdMissionType::AvoidStampede;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName = TEXT("Crowd Mission");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description = TEXT("Complete crowd-based objective");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float MissionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredCrowdSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float Progress = 0.0f;

    FQuest_CrowdMissionData()
    {
        MissionType = EQuest_CrowdMissionType::AvoidStampede;
        MissionName = TEXT("Survive the Stampede");
        Description = TEXT("Avoid being trampled by the migrating herd");
        TargetLocation = FVector::ZeroVector;
        MissionRadius = 2000.0f;
        RequiredCrowdSize = 20;
        TimeLimit = 180.0f;
        bIsActive = false;
        bIsCompleted = false;
        Progress = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CrowdBasedMissionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_CrowdBasedMissionSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void StartCrowdMission(EQuest_CrowdMissionType MissionType, FVector Location, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void CompleteMission(bool bSuccess = true);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    bool IsMissionActive() const { return CurrentMission.bIsActive; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    float GetMissionProgress() const { return CurrentMission.Progress; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    FQuest_CrowdMissionData GetCurrentMission() const { return CurrentMission; }

    // Crowd Interaction
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void UpdateCrowdMissionProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    bool CheckPlayerInCrowdArea(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    int32 GetCrowdCountInArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    TArray<AActor*> GetCrowdActorsInArea(FVector Center, float Radius);

    // Mission Types Implementation
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void ProcessAvoidStampedeMission(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void ProcessFollowMigrationMission(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void ProcessEscortMission(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void ProcessObservationMission(float DeltaTime);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdMissionStarted, EQuest_CrowdMissionType, MissionType, FVector, Location);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdMissionCompleted, bool, bSuccess, float, CompletionTime);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdMissionProgressUpdated, float, Progress);

    UPROPERTY(BlueprintAssignable, Category = "Crowd Mission Events")
    FOnCrowdMissionStarted OnCrowdMissionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Mission Events")
    FOnCrowdMissionCompleted OnCrowdMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Mission Events")
    FOnCrowdMissionProgressUpdated OnCrowdMissionProgressUpdated;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission State")
    FQuest_CrowdMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission State")
    float MissionStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission State")
    TArray<FQuest_CrowdMissionData> AvailableMissions;

    // Crowd System References
    UPROPERTY()
    class UCrowd_MassEntitySubsystem* CrowdSubsystem = nullptr;

    UPROPERTY()
    class UCrowd_FlockingBehavior* FlockingBehavior = nullptr;

    // Mission Logic
    void InitializeAvailableMissions();
    FQuest_CrowdMissionData CreateMissionData(EQuest_CrowdMissionType Type, const FString& Name, const FString& Desc);
    bool ValidateMissionCompletion();
    void BroadcastMissionEvents();

    // Crowd Detection
    bool IsPlayerInDanger(FVector PlayerLocation);
    float CalculateStampedeRisk(FVector PlayerLocation);
    bool IsPlayerFollowingMigration(FVector PlayerLocation);

private:
    FTimerHandle MissionUpdateTimer;
    float LastProgressUpdate = 0.0f;
    int32 ConsecutiveSuccessfulChecks = 0;
};