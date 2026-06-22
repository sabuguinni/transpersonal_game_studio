// CrowdSimulationManager.h
// Crowd & Traffic Simulation Agent #13
// Manages prehistoric crowd simulation: tribal NPCs, dinosaur herds, migration paths

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "CrowdSimulationManager.generated.h"

// Crowd agent state enum
UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Herding     UMETA(DisplayName = "Herding"),
    Resting     UMETA(DisplayName = "Resting")
};

// Crowd agent type
UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    TribalHuman     UMETA(DisplayName = "Tribal Human"),
    DinosaurHerd    UMETA(DisplayName = "Dinosaur Herd"),
    Scavenger       UMETA(DisplayName = "Scavenger"),
    Predator        UMETA(DisplayName = "Predator")
};

// Individual crowd agent data
USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 GroupID;

    FCrowd_AgentData()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , State(ECrowd_AgentState::Idle)
        , AgentType(ECrowd_AgentType::TribalHuman)
        , FearLevel(0.0f)
        , Energy(100.0f)
        , GroupID(0)
    {}
};

// Migration waypoint
USTRUCT(BlueprintType)
struct FCrowd_MigrationWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 WaypointIndex;

    FCrowd_MigrationWaypoint()
        : WorldPosition(FVector::ZeroVector)
        , Radius(300.0f)
        , WaypointIndex(0)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Maximum simultaneous agents (performance cap)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    // Migration corridor waypoints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FCrowd_MigrationWaypoint> MigrationWaypoints;

    // Active agents
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentData> ActiveAgents;

    // Threat radius — agents flee when player/predator within this range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float ThreatDetectionRadius;

    // Separation distance between agents (flocking)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float AgentSeparationDistance;

    // Herd cohesion strength
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float CohesionStrength;

    // Current active agent count
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    int32 ActiveAgentCount;

    // Spawn a group of tribal NPCs at location
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnTribalGroup(FVector CenterLocation, int32 GroupSize, int32 GroupID);

    // Spawn a dinosaur herd along migration path
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SpawnDinosaurHerd(FVector StartLocation, int32 HerdSize, int32 GroupID);

    // Trigger fear response — all agents within radius flee from source
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFearResponse(FVector ThreatLocation, float FearRadius, float FearIntensity);

    // Update flocking behavior (separation, alignment, cohesion)
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateFlockingBehavior(float DeltaTime);

    // Get nearest agent to a world position
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool GetNearestAgent(FVector WorldPosition, FCrowd_AgentData& OutAgent);

    // Clear all agents
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ClearAllAgents();

    // LOD: reduce simulation fidelity for distant agents
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void ApplyDistanceLOD(FVector PlayerLocation, float FullSimRadius, float LowSimRadius);

private:
    void UpdateAgentState(FCrowd_AgentData& Agent, float DeltaTime);
    FVector ComputeFlockingSeparation(const FCrowd_AgentData& Agent);
    FVector ComputeFlockingAlignment(const FCrowd_AgentData& Agent);
    FVector ComputeFlockingCohesion(const FCrowd_AgentData& Agent);
    FVector ComputeMigrationSteering(const FCrowd_AgentData& Agent);
    int32 GetNextWaypointIndex(int32 CurrentWaypoint) const;
};
