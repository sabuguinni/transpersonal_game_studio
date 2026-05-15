#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AgentSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AvoidanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance2 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float LODDistance3 = 5000.0f;

    FCrowd_SimulationSettings()
    {
        MaxCrowdAgents = 1000;
        SpawnRadius = 5000.0f;
        AgentSpeed = 150.0f;
        AvoidanceRadius = 100.0f;
        LODDistance1 = 1000.0f;
        LODDistance2 = 2000.0f;
        LODDistance3 = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FString GroupName = "DefaultGroup";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    int32 AgentCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    float GroupRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    bool bIsActive = true;

    FCrowd_AgentGroup()
    {
        GroupName = "DefaultGroup";
        AgentCount = 100;
        SpawnLocation = FVector::ZeroVector;
        GroupRadius = 500.0f;
        MovementSpeed = 150.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_AgentGroup> AgentGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    class UMassEntityConfigAsset* CrowdEntityConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Simulation")
    class UStaticMeshComponent* VisualizationMesh;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(const FCrowd_AgentGroup& AgentGroup);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdDestination(FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdAgents();

protected:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    UPROPERTY()
    bool bSimulationActive;

    UPROPERTY()
    float LastLODUpdateTime;

    UPROPERTY()
    FVector CurrentDestination;

    void UpdateAgentMovement(float DeltaTime);
    void ProcessAgentLOD();
    void HandleAgentCollisions();
    FVector CalculateFlockingBehavior(const FMassEntityHandle& Entity);
    FVector CalculateAvoidance(const FMassEntityHandle& Entity);
    FVector CalculateCohesion(const FMassEntityHandle& Entity);
    FVector CalculateAlignment(const FMassEntityHandle& Entity);
};