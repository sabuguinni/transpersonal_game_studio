#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class UCrowd_MassAgentComponent;
class ACrowd_SpawnPoint;
class ACrowd_Waypoint;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float LODDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsVisible = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 VisibleAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float CullingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float UpdateFrequencyHigh = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float UpdateFrequencyLow = 5.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd simulation core methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(int32 NumAgents, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehaviorState(ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterSpawnPoint(ACrowd_SpawnPoint* SpawnPoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterWaypoint(ACrowd_Waypoint* Waypoint);

    // LOD and performance methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAgentLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantAgents(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveAgentCount() const { return ActiveAgents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleAgentCount() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_SpawnConfiguration SpawnConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableCrowdSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableCulling = true;

protected:
    // Internal crowd data
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY()
    TArray<ACrowd_SpawnPoint*> SpawnPoints;

    UPROPERTY()
    TArray<ACrowd_Waypoint*> Waypoints;

    // Performance tracking
    float LastUpdateTime = 0.0f;
    int32 NextAgentID = 0;

    // Internal methods
    void CreateAgentAtLocation(const FVector& Location);
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    FVector CalculateSteeringForce(const FCrowd_AgentData& Agent);
    FVector FindNearestWaypoint(const FVector& Location);
    bool IsAgentVisible(const FCrowd_AgentData& Agent, const FVector& ViewerLocation);
};