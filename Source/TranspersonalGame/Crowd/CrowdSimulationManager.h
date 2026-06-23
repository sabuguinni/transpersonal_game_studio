// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// UE5 World Subsystem managing prehistoric crowd agents: herds, packs, flocks, tribes.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    HerbivoreHerd    UMETA(DisplayName = "Herbivore Herd"),
    PredatorPack     UMETA(DisplayName = "Predator Pack"),
    AerialFlock      UMETA(DisplayName = "Aerial Flock"),
    HumanTribe       UMETA(DisplayName = "Human Tribe"),
    ScavengerGroup   UMETA(DisplayName = "Scavenger Group")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Migrating   UMETA(DisplayName = "Migrating")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Near    UMETA(DisplayName = "Near — Full Sim"),
    Mid     UMETA(DisplayName = "Mid — Reduced Sim"),
    Far     UMETA(DisplayName = "Far — Minimal Sim"),
    Culled  UMETA(DisplayName = "Culled — No Sim")
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_Agent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::HerbivoreHerd;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Near;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float DesiredSpeed = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawn a group of agents of the given type around an origin point
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 SpawnHerd(ECrowd_AgentType AgentType, FVector Origin, int32 Count, float Radius = 500.0f);

    // Tick the simulation — call from GameMode or GameState each frame
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TickSimulation(float DeltaTime, const FVector& PlayerLocation);

    // Trigger a flee event — all agents within ThreatRadius will flee
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius = 1000.0f);

    // Update LOD levels based on player distance
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateAgentLOD(const FVector& PlayerLocation);

    // Query agents within a radius
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_Agent> GetAgentsInRadius(FVector Center, float Radius) const;

    // Get current active agent count
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceNear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

private:
    TArray<FCrowd_Agent> AgentPool;
    float TimeSinceLastTick;

    void TickAgent(FCrowd_Agent& Agent, float DeltaTime);
    float GetDefaultSpeedForType(ECrowd_AgentType Type) const;
};
