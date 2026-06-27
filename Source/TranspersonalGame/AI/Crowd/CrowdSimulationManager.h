#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CrowdSimulationManager.generated.h"

// Forward declarations
class UCrowdAgentComponent;
class UNavigationSystemV1;

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting"),
    Socializing UMETA(DisplayName = "Socializing")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HumanTribe      UMETA(DisplayName = "Human Tribe Member"),
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd Animal"),
    ScavengerPack   UMETA(DisplayName = "Scavenger Pack"),
    BirdFlock       UMETA(DisplayName = "Bird Flock")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::HumanTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType GroupType = ECrowd_AgentType::HumanTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector GroupCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<AActor*> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    AActor* Leader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionRadius = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsFleeing = false;
};

/**
 * ACrowdSimulationManager
 * Manages up to 50,000 crowd agents using UE5 Mass AI principles.
 * Handles tribal humans, herbivore herds, scavenger packs, and bird flocks.
 * Agents react to dinosaur presence, player actions, and environmental events.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgentsPerGroup = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveGroups = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationRadius = 10000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float UpdateIntervalSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceClose = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceMedium = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODDistanceFar = 10000.f;

    // --- Runtime State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 TotalActiveAgents = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 TotalActiveGroups = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_GroupData> ActiveGroups;

    // --- Spawn Points ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    TArray<FVector> TribeSpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Spawn")
    TArray<FVector> HerdSpawnPoints;

    // --- Blueprint Events ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnAgentStateChanged(AActor* Agent, ECrowd_AgentState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnGroupFleeTriggered(int32 GroupID, FVector ThreatLocation);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd|Events")
    void OnGroupReachedDestination(int32 GroupID);

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnCrowdGroup(ECrowd_AgentType AgentType, FVector SpawnCenter, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMassFlee(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerMassFlee_ByDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetGroupMigrationTarget(int32 GroupID, FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_GroupData GetGroupData(int32 GroupID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void DisbandGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSimulationEnabled(bool bEnabled);

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void SpawnDebugCrowdGroups();

    UFUNCTION(CallInEditor, Category = "Crowd|Debug")
    void ClearAllCrowdAgents();

private:
    // Internal update timer
    float TimeSinceLastUpdate = 0.f;
    bool bSimulationEnabled = true;
    int32 NextGroupID = 0;

    // Internal helpers
    void UpdateAllGroups(float DeltaTime);
    void UpdateGroupLOD(FCrowd_GroupData& Group, float DistanceToPlayer);
    void UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime);
    void UpdateGroupCohesion(FCrowd_GroupData& Group);
    void MoveAgentToward(AActor* Agent, FVector Target, float Speed, float DeltaTime);
    FVector GetRandomPointNearLocation(FVector Center, float Radius) const;
    float GetDistanceToPlayer(FVector Location) const;
    AActor* GetPlayerActor() const;
    void RemoveDeadAgentsFromGroup(FCrowd_GroupData& Group);
    int32 RegisterNewGroup(ECrowd_AgentType AgentType, FVector Center);
};
