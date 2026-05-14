#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../CrowdSimulation/Crowd_FlockingBehavior.h"
#include "../SharedTypes.h"
#include "Quest_CrowdMissionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    bool bIsEvacuationMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    bool bRequiresPlayerGuidance;

    FQuest_CrowdMissionData()
    {
        MissionName = TEXT("Unnamed Mission");
        Description = TEXT("No description");
        RequiredCrowdSize = 10;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        TimeLimit = 300.0f;
        bIsEvacuationMission = false;
        bRequiresPlayerGuidance = true;
    }
};

UENUM(BlueprintType)
enum class EQuest_CrowdMissionStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnCrowdMissionStatusChanged, int32, MissionID, EQuest_CrowdMissionStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuest_OnCrowdProgress, int32, MissionID, int32, CurrentCrowdSize, float, ProgressPercentage);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CrowdMissionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdMissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    int32 CreateCrowdMission(const FQuest_CrowdMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    bool StartCrowdMission(int32 MissionID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    bool CompleteCrowdMission(int32 MissionID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    bool FailCrowdMission(int32 MissionID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void AbandonCrowdMission(int32 MissionID);

    // Crowd Interaction
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void RegisterCrowdActor(AActor* CrowdActor, int32 MissionID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void UnregisterCrowdActor(AActor* CrowdActor, int32 MissionID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    int32 GetActiveCrowdCount(int32 MissionID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    float GetMissionProgress(int32 MissionID) const;

    // Player Guidance
    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void SetPlayerGuidanceTarget(int32 MissionID, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void EnablePlayerInfluence(int32 MissionID, float InfluenceRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Missions")
    void DisablePlayerInfluence(int32 MissionID);

    // Mission Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Missions")
    EQuest_CrowdMissionStatus GetMissionStatus(int32 MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Missions")
    bool IsMissionActive(int32 MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Missions")
    FQuest_CrowdMissionData GetMissionData(int32 MissionID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd Missions")
    TArray<int32> GetActiveMissionIDs() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Crowd Mission Events")
    FQuest_OnCrowdMissionStatusChanged OnMissionStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crowd Mission Events")
    FQuest_OnCrowdProgress OnCrowdProgress;

protected:
    // Mission Data Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<int32, FQuest_CrowdMissionData> ActiveMissions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<int32, EQuest_CrowdMissionStatus> MissionStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<int32, TArray<AActor*>> MissionCrowdActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission Data")
    TMap<int32, float> MissionTimers;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PlayerInfluenceStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDebugVisualization;

private:
    int32 NextMissionID;
    float UpdateTimer;

    // Internal Methods
    void UpdateMissionProgress(float DeltaTime);
    void CheckMissionCompletion(int32 MissionID);
    void CheckMissionFailure(int32 MissionID);
    void UpdatePlayerInfluence(int32 MissionID);
    void DrawDebugInfo(int32 MissionID);
    bool IsLocationInRadius(const FVector& Location, const FVector& Target, float Radius) const;
    void CleanupMission(int32 MissionID);
};