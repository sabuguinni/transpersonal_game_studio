#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_SimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance2 = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance3 = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float FlockingRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float CohesionWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FString GroupName = TEXT("DefaultGroup");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    int32 AgentCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    float GroupRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    float GroupMovementSpeed = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    BiomeType PreferredBiome = BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    bool bIsNomadic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Group")
    TArray<FVector> PatrolPoints;
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
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    FCrowd_SimulationSettings SimulationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    TArray<FCrowd_AgentGroup> AgentGroups;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Simulation")
    int32 ActiveAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Simulation")
    float CurrentSimulationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    bool bSimulationActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    bool bUsePerformanceLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float PerformanceThreshold = 16.67f; // Target 60 FPS

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void PauseSimulation();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void ResumeSimulation();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SpawnAgentGroup(const FCrowd_AgentGroup& GroupSettings);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void RemoveAgentGroup(const FString& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void UpdateLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    int32 GetActiveAgentCount() const { return ActiveAgentCount; }

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings);

protected:
    UFUNCTION()
    void InitializeMassEntity();

    UFUNCTION()
    void CreateAgentArchetypes();

    UFUNCTION()
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION()
    void UpdateAgentLOD();

    UFUNCTION()
    void ProcessPerformanceOptimization();

private:
    UMassEntitySubsystem* MassEntitySubsystem;
    TArray<FMassEntityHandle> SpawnedEntities;
    FMassArchetypeHandle CrowdArchetype;
    
    bool bIsInitialized = false;
    float LastFrameTime = 0.0f;
    int32 CurrentLODLevel = 0;
    
    TMap<FString, TArray<FMassEntityHandle>> GroupEntityMap;
    
    void CleanupEntities();
    void UpdateSimulationMetrics(float DeltaTime);
    FVector CalculateFlockingForce(const FMassEntityHandle& Entity, const TArray<FMassEntityHandle>& NearbyEntities);
};