#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HerbivorePrey   UMETA(DisplayName = "HerbivorePrey"),
    CarnivoreHunter UMETA(DisplayName = "CarnivoreHunter"),
    HumanTribe      UMETA(DisplayName = "HumanTribe"),
    Scavenger       UMETA(DisplayName = "Scavenger")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::HerbivorePrey;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Health = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 1500.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdGroupID = -1;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CentroidLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TArray<int32> MemberAgentIDs;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType GroupType = ECrowd_AgentType::HerbivorePrey;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsMigrating = false;
};

UCLASS(ClassGroup = "TranspersonalGame", BlueprintType, Blueprintable, meta = (DisplayName = "Crowd Simulation Manager"))
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SimulationRadius = 10000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickIntervalSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AlignmentRadius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float ForageSpeed = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationSpeed = 300.f;

    // --- Runtime State ---
    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_AgentData> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_HerdGroup> HerdGroups;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    float AccumulatedTime = 0.f;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeCrowd(int32 NumHerbivores, int32 NumCarnivores, int32 NumHumans);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius, ECrowd_AgentType AffectedType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void StartMigration(int32 GroupID, FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetAgentState(int32 AgentID, ECrowd_AgentState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_AgentData GetAgentData(int32 AgentID) const;

private:
    void StepSimulation(float DeltaTime);
    void UpdateAgent(FCrowd_AgentData& Agent, float DeltaTime);
    FVector ComputeFlockingForce(const FCrowd_AgentData& Agent) const;
    FVector ComputeSeparation(const FCrowd_AgentData& Agent) const;
    FVector ComputeCohesion(const FCrowd_AgentData& Agent) const;
    FVector ComputeAlignment(const FCrowd_AgentData& Agent) const;
    void UpdateHerdCentroids();
    int32 NextAgentID = 0;
    int32 NextGroupID = 0;
};
