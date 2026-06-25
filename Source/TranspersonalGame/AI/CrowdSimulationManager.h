#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

/**
 * UCrowdSimulationManager
 * Agent #13 — Crowd & Traffic Simulation
 *
 * Manages prehistoric herd behavior: migration corridors, flocking (separation/cohesion/alignment),
 * LOD-aware agent updates, and threat-response scatter for up to 50 agents per herd.
 *
 * Usage: Attach to a CrowdDirector actor in the level. Tag herd members with "CrowdAgent".
 * Call AddMigrationWaypoint() to define the migration path.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent), DisplayName="Crowd Simulation Manager")
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---

    /** Maximum number of agents in this herd */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdSize;

    /** Base movement speed for migration (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float MigrationSpeed;

    /** Radius within which agents influence each other for flocking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float FlockingRadius;

    /** Minimum separation distance between agents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius;

    /** Distance at which full LOD detail is used */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceClose;

    /** Distance at which mid LOD is applied */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceMid;

    /** Distance at which far LOD is applied (beyond = hidden) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LODDistanceFar;

    /** Radius within which player presence triggers herd scatter */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float ThreatResponseRadius;

    // --- Runtime State ---

    /** All active herd agents */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    TArray<AActor*> HerdAgents;

    /** Migration waypoints defining the corridor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FVector> MigrationWaypoints;

    /** Whether migration is currently active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    bool bHerdMigrationActive;

    /** Current target waypoint index */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State")
    int32 CurrentWaypointIndex;

    // --- Blueprint-callable API ---

    /** Add a waypoint to the migration corridor */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void AddMigrationWaypoint(FVector Waypoint);

    /** Enable or disable herd migration */
    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void SetHerdMigrationActive(bool bActive);

    /** Returns number of currently visible (non-LOD-hidden) agents */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd|State")
    int32 GetActiveAgentCount() const;

    /** Force re-scan of world for CrowdAgent-tagged actors */
    UFUNCTION(BlueprintCallable, Category = "Crowd|State")
    void InitializeHerd();

private:
    /** Compute flocking force (separation + cohesion) for a single agent */
    FVector ComputeFlockingForce(AActor* Agent) const;

    /** Compute geometric centroid of the herd */
    FVector ComputeHerdCentroid() const;

    /** Move all agents toward current waypoint with flocking */
    void UpdateHerdMigration(float DeltaTime);

    /** Adjust skeletal mesh tick rates based on player distance */
    void UpdateLODForAgents();

    /** Scatter herd if player is within ThreatResponseRadius */
    void CheckThreatResponse();
};
