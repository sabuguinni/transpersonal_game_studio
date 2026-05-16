#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "MassEntityConfigAsset.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Crowd_MassSimulationManager.generated.h"

// Forward declarations
class UMassEntitySubsystem;
class UMassSpawnerSubsystem;
class UBehaviorTree;

UENUM(BlueprintType)
enum class ECrowd_CrowdBehaviorType : uint8
{
    Wandering       UMETA(DisplayName = "Wandering"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Following       UMETA(DisplayName = "Following"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Resting         UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_CrowdDensity : uint8
{
    Sparse          UMETA(DisplayName = "Sparse (1-10 entities)"),
    Medium          UMETA(DisplayName = "Medium (10-50 entities)"),
    Dense           UMETA(DisplayName = "Dense (50-200 entities)"),
    Massive         UMETA(DisplayName = "Massive (200+ entities)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntitySpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float SpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    int32 EntityCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    ECrowd_CrowdBehaviorType BehaviorType = ECrowd_CrowdBehaviorType::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    ECrowd_CrowdDensity DensityLevel = ECrowd_CrowdDensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float MovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    bool bEnableLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float LODDistance2 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Parameters")
    float LODDistance3 = 5000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> WaypointPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector CurrentTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseNavMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bAvoidOtherEntities = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AvoidanceRadius = 150.0f;
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

    // Core Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassEntitySystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(const FCrowd_EntitySpawnParams& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnEntitiesInRadius(const FVector& Location, float Radius);

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehavior(ECrowd_CrowdBehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdDensity(ECrowd_CrowdDensity NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdTarget(const FVector& TargetLocation);

    // Pathfinding and Navigation
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdatePathfinding();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetWaypointPath(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnableNavMeshPathfinding(bool bEnable);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float Distance1, float Distance2, float Distance3);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCurrentPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void EnablePerformanceOptimization(bool bEnable);

    // Combat Integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void TriggerCrowdFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdCombatMode(bool bCombatMode);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation", CallInEditor = true)
    void DebugDrawCrowdInfo();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation", CallInEditor = true)
    void ToggleDebugVisualization();

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualizationMesh;

    // Mass Entity System References
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    class UMassSpawnerSubsystem* MassSpawnerSubsystem;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    FCrowd_EntitySpawnParams DefaultSpawnParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    FCrowd_PathfindingData PathfindingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    int32 MaxEntityCount = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    float TickInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    bool bAutoSpawnOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Configuration")
    bool bEnableDebugVisualization = false;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    int32 CurrentEntityCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    ECrowd_CrowdBehaviorType CurrentBehaviorType = ECrowd_CrowdBehaviorType::Wandering;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bIsCombatMode = false;

    // Performance Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PerformanceSampleCount = 0;

private:
    // Internal Management
    void UpdatePerformanceMetrics(float DeltaTime);
    void ProcessEntityBehaviors();
    void UpdateEntityLOD();
    void HandleCombatEvents();
    
    // Timers
    float PathfindingUpdateTimer = 0.0f;
    float LODUpdateTimer = 0.0f;
    float BehaviorUpdateTimer = 0.0f;
    
    // Entity Storage
    TArray<FMassEntityHandle> ManagedEntities;
    TMap<FMassEntityHandle, FCrowd_PathfindingData> EntityPathfindingData;
};