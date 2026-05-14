#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassCommonTypes.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AgentSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SeparationDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    bool bEnableAvoidance = true;

    FCrowd_SimulationSettings()
    {
        MaxCrowdAgents = 1000;
        SpawnRadius = 5000.0f;
        AgentSpeed = 150.0f;
        SeparationDistance = 100.0f;
        bEnableFlocking = true;
        bEnableAvoidance = true;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_AgentGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FString GroupName = "DefaultGroup";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    int32 AgentCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    float GroupRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    TArray<FVector> PatrolPoints;

    FCrowd_AgentGroup()
    {
        GroupName = "DefaultGroup";
        AgentCount = 50;
        SpawnLocation = FVector::ZeroVector;
        GroupRadius = 500.0f;
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
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_AgentGroup> AgentGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    TSoftObjectPtr<UMassEntityConfigAsset> CrowdEntityConfig;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnAgentGroup(const FCrowd_AgentGroup& GroupSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ResumeSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllAgents();

private:
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    TArray<FMassEntityHandle> SpawnedEntities;
    
    bool bSimulationActive = false;
    bool bSimulationPaused = false;
    float SimulationTime = 0.0f;

    void InitializeMassSubsystems();
    void CreateEntityArchetype();
    void SpawnCrowdEntities(const FCrowd_AgentGroup& GroupSettings);
    void UpdateAgentBehavior(float DeltaTime);
    void CleanupEntities();
};