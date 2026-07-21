#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "World_PhysicsIntegratedTerrainGenerator.generated.h"

// Physics-integrated terrain chunk for performance optimization
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PhysicsTerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    int32 PhysicsComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    bool bHasCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    float PerformanceCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Terrain")
    EBiomeType BiomeType;

    FWorld_PhysicsTerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);
        PhysicsComplexity = 1;
        bHasCollision = true;
        PerformanceCost = 1.0f;
        BiomeType = EBiomeType::Forest;
    }
};

// Performance settings for physics-integrated terrain
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PhysicsTerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsComplexityBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CollisionLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsConsolidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime;

    FWorld_PhysicsTerrainSettings()
    {
        MaxPhysicsComplexityBudget = 100.0f;
        MaxActiveChunks = 25;
        CollisionLODDistance = 5000.0f;
        bEnablePhysicsConsolidation = true;
        TargetFrameTime = 16.67f; // 60 FPS
    }
};

/**
 * Physics-integrated terrain generator that works with Agent #3's physics consolidation
 * and Agent #4's performance optimization systems for Milestone 1 "WALK AROUND"
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(WorldGeneration))
class TRANSPERSONALGAME_API UWorld_PhysicsIntegratedTerrainGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_PhysicsIntegratedTerrainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core terrain generation with physics integration
    UFUNCTION(BlueprintCallable, Category = "Physics Terrain")
    void GeneratePhysicsOptimizedTerrain();

    UFUNCTION(BlueprintCallable, Category = "Physics Terrain")
    void CreateTerrainChunk(const FVector& Location, const FVector& Size, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Physics Terrain")
    void OptimizeTerrainPhysics();

    // Performance integration with Agent #4
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateTerrainPerformanceCost() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinPerformanceBudget() const;

    // Physics consolidation integration with Agent #3
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsConsolidator();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsComplexity(float NewComplexity);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ConsolidateTerrainCollision();

    // Milestone 1 specific functionality
    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void CreateWalkableTerrainForMilestone();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void ValidateTerrainWalkability();

    UFUNCTION(BlueprintCallable, Category = "Milestone")
    void EnsurePerformanceTargets();

protected:
    // Terrain chunk management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Chunks")
    TArray<FWorld_PhysicsTerrainChunk> ActiveTerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorld_PhysicsTerrainSettings TerrainSettings;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float CurrentPerformanceCost;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastFrameTime;

    // Physics integration state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    bool bPhysicsConsolidatorConnected;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    float PhysicsComplexityBudget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
    int32 ConsolidatedCollisionObjects;

private:
    // Internal terrain generation
    void GenerateBaseHeightmap();
    void ApplyBiomeModifications();
    void CreateCollisionGeometry();
    void OptimizeForPerformance();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyLODOptimizations();

    // Physics integration helpers
    void ConnectToPhysicsConsolidator();
    void UpdatePhysicsBudget();
    void ConsolidateChunkPhysics();
};