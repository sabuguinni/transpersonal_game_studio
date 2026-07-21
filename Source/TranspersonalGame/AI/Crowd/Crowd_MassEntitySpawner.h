#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySpawner.generated.h"

class UMassEntitySubsystem;
class UCrowd_MassEntityConfig;
struct FCrowd_MovementFragment;
struct FCrowd_BehaviorFragment;

/**
 * Spawner for Mass Entity crowd simulation
 * Creates and manages up to 50,000 crowd entities
 */
UCLASS(BlueprintType, meta = (DisplayName = "Crowd Mass Entity Spawner"))
class TRANSPERSONALGAME_API ACrowd_MassEntitySpawner : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntitySpawner();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Spawning interface
    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SpawnCrowdEntities(int32 NumEntities);
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void DespawnAllEntities();
    
    UFUNCTION(BlueprintCallable, Category = "Crowd Spawning")
    void SetCrowdDensity(float DensityPercentage);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Config")
    TObjectPtr<UCrowd_MassEntityConfig> CrowdConfig;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    int32 MaxCrowdEntities = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float SpawnRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    float MinSpawnDistance = 200.0f;

    // Spawn areas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    TArray<FVector> SpawnPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    TArray<FVector> WaypointLocations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawning")
    bool bAutoGenerateWaypoints = true;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 EntitiesPerBatch = 1000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float BatchSpawnDelay = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceScaling = true;

    // Status
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    int32 CurrentEntityCount = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    float CurrentDensityPercentage = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bIsSpawning = false;

protected:
    // Internal spawning functions
    void SpawnEntityBatch(int32 BatchSize, int32 BatchIndex);
    void GenerateSpawnPoints();
    void GenerateWaypoints();
    FVector GetRandomSpawnLocation() const;
    FTransform GetSpawnTransform(const FVector& Location) const;
    
    // Entity configuration
    void ConfigureEntityFragments(FMassEntityHandle Entity, const FTransform& SpawnTransform);
    void SetupMovementFragment(FMassEntityHandle Entity, const FTransform& SpawnTransform);
    void SetupBehaviorFragment(FMassEntityHandle Entity);
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    bool CanSpawnMoreEntities() const;

private:
    // Mass Entity system references
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;
    
    // Entity archetype for crowd entities
    FMassArchetypeHandle CrowdArchetype;
    
    // Spawned entities tracking
    TArray<FMassEntityHandle> SpawnedEntities;
    
    // Batch spawning state
    FTimerHandle BatchSpawnTimer;
    int32 CurrentBatchIndex = 0;
    int32 TargetEntityCount = 0;
    
    // Performance tracking
    float LastPerformanceUpdate = 0.0f;
    float AverageFrameTime = 0.0f;
    
    // Constants
    static constexpr float PerformanceUpdateInterval = 1.0f;
    static constexpr float MaxFrameTimeThreshold = 0.033f; // 30 FPS
};