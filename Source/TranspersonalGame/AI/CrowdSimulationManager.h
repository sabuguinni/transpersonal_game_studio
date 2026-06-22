#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// Agent #13 — Crowd & Traffic Simulation
// Manages prehistoric herd migration and predator pack patrol using waypoint-driven crowd AI.
// Supports up to MaxActiveAgents simultaneous agents with LOD-based update throttling.

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore Herd"),
    Predator    UMETA(DisplayName = "Predator Pack"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
    Neutral     UMETA(DisplayName = "Neutral")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TWeakObjectPtr<AActor> AgentActor;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Neutral;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsFleeing = false;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdCohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PredatorFleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float WaypointReachThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdMigrationInterval = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PackPatrolInterval = 15.0f;

    // --- Runtime State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    bool bSimulationActive;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    int32 ActiveAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FCrowd_AgentData> RegisteredAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FVector> HerdWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|State")
    TArray<FVector> PackWaypoints;

    // --- Public API ---

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterAgent(AActor* Agent, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector GetCurrentHerdTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector GetCurrentPackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void CollectWaypointsFromLevel();

private:
    void UpdateHerdBehavior(float DeltaTime);
    void UpdatePackBehavior(float DeltaTime);
    void EnforceCrowdCap();

    int32 CurrentHerdWaypointIndex = 0;
    int32 CurrentPackWaypointIndex = 0;
    float HerdWaypointTimer = 0.0f;
    float PackPatrolTimer = 0.0f;
};
