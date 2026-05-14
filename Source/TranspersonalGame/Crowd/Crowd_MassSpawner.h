#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassEntitySubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Crowd_MassSpawner.generated.h"

/**
 * Mass Entity spawner for crowd simulation
 * Spawns and manages large numbers of NPCs using UE5 Mass Entity framework
 * Supports up to 50,000 simultaneous agents with LOD optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Spawning Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    int32 MaxAgents = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float AgentSpacing = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bAutoSpawn = true;

    // Agent Types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Types")
    TArray<ECrowd_AgentType> AgentTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Types")
    TArray<float> TypeWeights;

    // Visual Representation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* HumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* AnimalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<UMaterialInterface*> AgentMaterials;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 8000.0f;

    // Behavior Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FlockingRadius = 500.0f;

    // Mass Entity System
    UPROPERTY()
    FMassEntityHandle EntityTemplate;

    UPROPERTY()
    TArray<FMassEntityHandle> SpawnedEntities;

    // Spawning Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SpawnCrowdAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SetAgentCount(int32 NewCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    int32 GetActiveAgentCount() const;

    // Configuration Functions
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void UpdateLODDistances(float High, float Medium, float Low);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetMovementParameters(float Speed, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void EnableFlocking(bool bEnable, float Radius = 500.0f);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugSpawnTestAgents();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawDebugInfo();

private:
    // Internal spawning logic
    void SpawnAgentAtLocation(const FVector& Location, ECrowd_AgentType AgentType);
    FVector GetRandomSpawnLocation() const;
    ECrowd_AgentType SelectRandomAgentType() const;
    
    // Mass Entity management
    void InitializeMassEntitySystem();
    void CreateEntityTemplate();
    void RegisterEntityProcessors();
    
    // LOD management
    void UpdateAgentLOD();
    ECrowd_LODLevel CalculateLODLevel(const FVector& AgentLocation) const;
    
    // Performance tracking
    float LastSpawnTime;
    int32 FrameSpawnCount;
    static constexpr int32 MaxSpawnsPerFrame = 100;
    
    // Cached references
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;
    
    UPROPERTY()
    class APawn* PlayerPawn;
};