// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival crowd AI: human tribes + dinosaur herds
// Up to 500 agents with LOD, boids flocking, flee response

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

// ── Enums (global scope — RULE 1) ─────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Near    UMETA(DisplayName = "Near — Full Simulation"),
    Mid     UMETA(DisplayName = "Mid — Reduced Simulation"),
    Far     UMETA(DisplayName = "Far — Minimal Simulation"),
    Culled  UMETA(DisplayName = "Culled — No Simulation")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Herding     UMETA(DisplayName = "Herding — Boids Flocking"),
    Foraging    UMETA(DisplayName = "Foraging — Resource Gathering"),
    Resting     UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HumanTribeMember    UMETA(DisplayName = "Human Tribe Member"),
    HumanForager        UMETA(DisplayName = "Human Forager"),
    DinosaurHerd        UMETA(DisplayName = "Dinosaur Herd Member"),
    RaptorPack          UMETA(DisplayName = "Raptor Pack Member"),
    MigrationLeader     UMETA(DisplayName = "Migration Leader")
};

// ── Structs (global scope — RULE 1) ───────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::HumanTribeMember;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Near;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector WanderTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float TickRate = 0.1f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float IdleTimer = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float IdleDuration = 5.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ForageTimer = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ForageInterval = 8.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float HerdRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float SeparationRadius = 120.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeRadius = 1500.0f;
};

// ── UCrowdSimulationManager ────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent management
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(UPARAM(ref) FCrowd_AgentData& AgentData);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(int32 AgentID);

    // LOD system
    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateLOD(const FVector& PlayerLocation);

    // Behavior update
    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void UpdateAgentBehavior(UPARAM(ref) FCrowd_AgentData& Agent, float DeltaTime);

    // Threat response — triggers flee for all agents in radius
    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void TriggerFleeResponse(AActor* ThreatActor, float ThreatRadius);

    // Queries
    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    TArray<FCrowd_AgentData> GetAgentsInRadius(const FVector& Center, float Radius) const;

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceNear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> AgentPool;

    int32 NextAgentID = 0;

    void UpdateIdleBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateWanderBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateFleeBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateHerdBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateForageBehavior(FCrowd_AgentData& Agent, float DeltaTime);

    FVector GetRandomWanderTarget(const FCrowd_AgentData& Agent, float Radius = 600.0f);
};
