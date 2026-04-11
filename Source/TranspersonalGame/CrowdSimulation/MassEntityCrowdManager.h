#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "MassEntityCrowdManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrowdSimulation, Log, All);

/**
 * Mass Entity-based crowd simulation manager
 * Handles up to 50,000 simultaneous crowd agents using UE5's Mass Entity framework
 * Supports LOD-based rendering, pathfinding, and behavioral states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AMassEntityCrowdManager : public AActor
{
    GENERATED_BODY()

public:
    AMassEntityCrowdManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // Mass Entity Framework Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mass Entity")
    class UMassSimulationSubsystem* MassSimulationSubsystem;

    // Crowd Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings", meta = (ClampMin = "1", ClampMax = "50000"))
    int32 MaxCrowdAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AgentSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AgentRadius = 50.0f;

    // LOD Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumLODDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowLODDistance = 5000.0f;

    // Mesh Assets for different LODs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    class UStaticMesh* HighLODMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    class UStaticMesh* MediumLODMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    class UStaticMesh* LowLODMesh;

    // Crowd Behavior States
    UENUM(BlueprintType)
    enum class ECrowdAgentState : uint8
    {
        Idle,
        Walking,
        Running,
        Gathering,
        Fleeing,
        Following
    };

    // Mass Entity Archetype
    FMassArchetypeHandle CrowdArchetype;

    // Active crowd entities
    TArray<FMassEntityHandle> CrowdEntities;

    // Spawn points for crowd generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points")
    TArray<FVector> SpawnPoints;

    // Destination points for crowd movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destinations")
    TArray<FVector> DestinationPoints;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void SpawnCrowdAgents(int32 NumAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void SetCrowdDestination(const FVector& NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void TriggerCrowdPanic(const FVector& PanicCenter, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void AddSpawnPoint(const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Control")
    void AddDestinationPoint(const FVector& DestinationLocation);

    UFUNCTION(BlueprintPure, Category = "Crowd Info")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd Info")
    float GetCrowdDensity(const FVector& Location, float Radius) const;

private:
    void InitializeMassEntityFramework();
    void CreateCrowdArchetype();
    void UpdateCrowdLOD(float DeltaTime);
    void UpdateCrowdBehavior(float DeltaTime);
    
    // Performance monitoring
    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 1.0f;
    
    // Crowd statistics
    int32 HighLODAgentCount = 0;
    int32 MediumLODAgentCount = 0;
    int32 LowLODAgentCount = 0;
};