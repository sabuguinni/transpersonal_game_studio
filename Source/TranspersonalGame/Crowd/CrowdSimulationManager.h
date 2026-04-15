#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "../SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_BehaviorState CurrentBehavior = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float AlertRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 GroupID = 0;

    FCrowd_AgentData()
    {
        AgentType = ECrowd_AgentType::Gatherer;
        CurrentBehavior = ECrowd_BehaviorState::Idle;
        TargetLocation = FVector::ZeroVector;
        MovementSpeed = 150.0f;
        AlertRadius = 500.0f;
        GroupID = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 MaxAgents = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::Settlement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    TArray<ECrowd_AgentType> AllowedAgentTypes;

    FCrowd_SpawnZone()
    {
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxAgents = 50;
        ZoneType = ECrowd_ZoneType::Settlement;
        AllowedAgentTypes = {ECrowd_AgentType::Gatherer, ECrowd_AgentType::Hunter, ECrowd_AgentType::Crafter};
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core crowd simulation properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxCrowdAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SimulationRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableMassEntity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_SpawnZone> SpawnZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> ActiveAgents;

    // LOD system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD0_Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD1_Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float LOD2_Distance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD System")
    float CullDistance = 8000.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAgentsPerFrame = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    // Core simulation methods
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(const FCrowd_SpawnZone& SpawnZone, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdAlert(const FVector& AlertLocation, float AlertRadius, ECrowd_AlertType AlertType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> GetCrowdPositions() const;

    // Mass Entity integration
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetupMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnMassEntities(int32 Count, const FVector& Location);

protected:
    // Internal simulation state
    float LastUpdateTime = 0.0f;
    int32 CurrentAgentIndex = 0;
    bool bSimulationActive = false;

    // Mass Entity system
    UMassEntitySubsystem* MassEntitySubsystem = nullptr;
    FMassEntityHandle EntityTemplate;

    // Internal methods
    void UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime);
    void ProcessAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    void HandleAgentInteractions(FCrowd_AgentData& Agent);
    ECrowd_BehaviorState DetermineBehaviorState(const FCrowd_AgentData& Agent);
    FVector FindNearestWaypoint(const FVector& CurrentLocation);
    bool IsLocationSafe(const FVector& Location);
};